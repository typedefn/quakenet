/*
 * BspParser.cpp
 *
 *  Created on: Dec 18, 2021
 *      Author: mastakillah
 */

#include "BspParser.hpp"

BspParser::BspParser() {
  modelBytes = nullptr;
  paletteHandle = 0;
}

BspParser::~BspParser() {
  if (modelBytes) {
    delete[] modelBytes;
  }

  fclose(paletteHandle);
  fclose(pak.handle);
}

Model BspParser::loadModel(const std::string &mapName) {
  PackFile packFile;

  std::stringstream ss;
  ss << mapName;

  paletteHandle = fopen("../resources/textures/palette.lmp", "rb");
  if (paletteHandle == nullptr) {
    std::stringstream ss;
    ss << "Failed to open file palette.lmp errno = " << errno;
    throw std::runtime_error(ss.str());
  }

  fread(palettes, 1, 768, paletteHandle);

  unsigned *t;
  unsigned r, g, b, v;
  byte *pal = &palettes[0];
  for (int i = 0; i < 256; i++) {
    r = pal[0]; // * (2.0 / 1.5); if (r > 255) r = 255;
    g = pal[1]; // * (2.0 / 1.5); if (g > 255) g = 255;
    b = pal[2]; //* (2.0 / 1.5); if (b > 255) b = 255;
    pal += 3;
    v = Utility::littleLong((255 << 24) + (r << 0) + (g << 8) + (b << 16));
    table[i] = v;
  }

  table[255] = 0;

  pak.handle = fopen(ss.str().c_str(), "r");
  if (pak.handle == nullptr) {
    std::stringstream errss;
    errss << "Failed to open " << ss.str() << " with errno " << errno;
    throw std::runtime_error(errss.str());
  }

  fseek(pak.handle, 0L, SEEK_END);
  // calculating the size of the file
  packFile.filelen = ftell(pak.handle);
  packFile.filepos = 0;
  strcpy(model.name, mapName.c_str());
  modelBytes = new byte[sizeof(byte) * (packFile.filelen + 1)];

  fseek(pak.handle, packFile.filepos, SEEK_SET);
  fread(modelBytes, 1, packFile.filelen, pak.handle);
  int modType = (modelBytes[0] | (modelBytes[1] << 8) | (modelBytes[2] << 16) | (modelBytes[3] << 24));

  loadBrushModel();

  std::vector<glm::vec3> localVertex;
  std::vector<uint32_t> localIndex;
  std::vector<glm::vec2> localUVs;

  int baseIndex = 0;

  for (auto &s : model.surfaces) {
    Polygon *p = &s.polys;
    baseIndex = s.firstedge;

    const int numverts = p->verts.size();
    const int vertexCount = p->numverts - 2;

    for (const auto &v : p->verts) {
      localVertex.push_back(v.position);
    }

    glm::vec2 u = glm::vec2(s.texinfo.vecs[0][0], s.texinfo.vecs[0][1]);
    glm::vec2 v = glm::vec2(s.texinfo.vecs[1][0], s.texinfo.vecs[1][1]);

    localUVs.push_back(u);
    localUVs.push_back(v);

    for (int i = 0; i < vertexCount; ++i) {
      localIndex.push_back(baseIndex);
      localIndex.push_back(baseIndex + i + 1);
      localIndex.push_back(baseIndex + i + 2);
    }

  }

  model.vertexes = localVertex;
  model.edges = localIndex;
  model.uvs = localUVs;

  LOG << "Done Loading surfaces vertex size = " << localVertex.size() << " index size = " << localIndex.size() << " local uvs size = " << localUVs.size();
  return model;
}

std::vector<glm::vec3> BspParser::getTriangulatedVertices() {
  std::vector<glm::vec3> vertices;

  for (uint32_t e : model.edges) {
    glm::vec3 vert = model.vertexes.at(e);
    vertices.push_back(vert);
  }

  return vertices;
}

std::vector<glm::vec3> BspParser::getTriangulatedNormals() {
  std::vector<glm::vec3> vertices = this->getTriangulatedVertices();
  std::vector<glm::vec3> normals;

  for (uint32_t i = 0; i < vertices.size(); i += 3) {
    glm::vec3 p0 = vertices[i];
    glm::vec3 p1 = vertices[(i + 1) % vertices.size()];
    glm::vec3 p2 = vertices[(i + 2) % vertices.size()];
    glm::vec3 A = p1 - p0;
    glm::vec3 B = p2 - p1;

    glm::vec3 normal = (glm::cross(B, A));
    normals.push_back(normal);

    float delta = glm::dot(normal, glm::vec3(0, 1, 0));

    /*
     * delta <= 0 indicates walls
     * delta > 0 indicates walkable floors
     */
  }

  return normals;
}

std::vector<std::vector<int>> BspParser::generatePov() {
  navNodes.clear();
  int nodeIndex = 0; 
  for (size_t i = 0; i < model.surfaces.size(); ++i) {
    auto surface = model.surfaces.at(i);
    std::string textureName = surface.texinfo.texture.name;
  
    if (textureName == "trigger") {
      continue;
    }    

    Polygon * p = &surface.polys;

    for (size_t j = 0; j < p->verts.size(); ++j) {
      glm::vec3 p0 = p->verts.at(j).position;
      NavNode node { nodeIndex, p0 };
      navNodes.push_back(node);
      nodeIndex++;
    }
  }

  std::vector<int> row(navNodes.size(), 0);
  std::vector<std::vector<int>> edges(navNodes.size(), row);

  for (int i = 0; i < navNodes.size(); i++) {
    NavNode node = navNodes.at(i);

    for (int j = 0; j < navNodes.size(); j++) {
      NavNode node2 = navNodes.at(j);

      if (i != j) {
        if (edgeIsValid(node.position, node2.position)) {
          edges[i][j] = j;
        }
      }
    }
  }

  return edges;
}

bool BspParser::edgeIsValid(glm::vec3 sourcePosition, glm::vec3 destinationPosition) {
  float c = 25;

  glm::vec3 tmin = glm::vec3(-c, -c, -c);
  glm::vec3 tmax = glm::vec3(c, c, c);

  Box destinationBox(tmin + destinationPosition, tmax + destinationPosition);

  float distanceToDestination = glm::distance(sourcePosition, destinationPosition);

  glm::vec3 direction = destinationPosition - sourcePosition;

  Ray ray(sourcePosition, glm::normalize(direction));

  float t2 = 0.0;
  bool destinationIntersected = destinationBox.intersectV2(ray, &t2);
  float offset = 999999;
  float smallestT1 = offset;

  for (auto & surface : model.surfaces) {
    std::string textureName = surface.texinfo.texture.name;
 
    if (textureName == "trigger") {
      continue;
    }

    glm::vec3 surfMin(offset);
    glm::vec3 surfMax(-offset);

    Polygon * p = &surface.polys;

    for (size_t j = 0; j < p->verts.size(); ++j) {
      glm::vec3 p0 = p->verts.at(j).position;
      MathUtil::instance()->findMin(p0, surfMin);
      MathUtil::instance()->findMax(p0, surfMax);
    }

    Box box(surfMin, surfMax);
    box.setCenter((box.getMin() + box.getMax()) / 2.0f);

    glm::vec3 po = box.getCenter();

    float dist = glm::distance(sourcePosition, po);
    float t = 0.0;

    bool rayIntersected = box.intersectV2(ray, &t);

    if (destinationIntersected && rayIntersected && smallestT1 > t) {
      smallestT1 = t;
    }
  }

  return t2 < smallestT1;
}

void BspParser::loadBrushModel() {
  int i, j;
  int bsp2;
  Header *header;
  Model *bm;
  float radius;

  header = (Header*) modelBytes;
  model.bspversion = Utility::littleLong(header->version);
  LOG << "Detected BSP version " << model.bspversion;

  switch (model.bspversion) {
    case BSPVERSION:
      bsp2 = false;
      break;
    case BSP2VERSION_2PSB:
      bsp2 = 1; //first iteration
      break;
    case BSP2VERSION_BSP2:
      bsp2 = 2; //sanitised revision
      break;
    default: {
      LOG << "loadBrushModel: " << model.name << "has wrong version number (" << model.bspversion << " should be " << BSPVERSION << ")";
      return;
    }
  }

  loadVertexData(&header->lumps[LUMP_VERTEXES]);
  loadEdges(&header->lumps[LUMP_EDGES]);
  loadSurfaceEdges(&header->lumps[LUMP_SURFEDGES]);
  loadTextures(&header->lumps[LUMP_TEXTURES]);
  // loadLighting
  // loadPlanes
  loadTexInfo(&header->lumps[LUMP_TEXINFO]);
  loadFaces(&header->lumps[LUMP_FACES]);
  // loadMarkSurfaces
  loadEntities(&header->lumps[LUMP_ENTITIES]);
  loadSubmodels(&header->lumps[LUMP_MODELS]);
}

void BspParser::loadVertexData(Lump *l) {
  Vertex *in;
  int i, count;

  in = (Vertex*) (modelBytes + l->fileofs);
  if (l->filelen % sizeof(*in)) {
    LOG << "loadVertexData: funny lump size in " << model.name;
    return;
  }

  count = l->filelen / sizeof(*in);

  for (i = 0; i < count; i++, in++) {
    glm::vec3 v;
    v.x = Utility::littleFloat(in->point[0]);
    v.y = Utility::littleFloat(in->point[2]);
    v.z = Utility::littleFloat(in->point[1]);
    model.vertexes.push_back(v);
  }

}

void BspParser::loadEdges(Lump *l) {
  int i, count;

  Edge *in = (Edge*) (modelBytes + l->fileofs);

  if (l->filelen % sizeof(*in)) {
    LOG << "loadEdges: funny lump size in " << model.name;
    return;
  }

  count = l->filelen / sizeof(*in);

  for (i = 0; i < count; i++, in++) {
    ModelEdge edge;
    edge.v[0] = Utility::littleLong(in->v[0]);
    edge.v[1] = Utility::littleLong(in->v[1]);

    model.medges.push_back(edge);
  }

}

void BspParser::loadSurfaceEdges(Lump *l) {
  int i, count;
  int *in;
  std::vector<int> out;

  in = (int*) (modelBytes + l->fileofs);
  if (l->filelen % sizeof(*in)) {
    LOG << "loadSurfaceEdges: funny lump size in " << model.name;
    return;
  }

  count = l->filelen / sizeof(*in);

  for (i = 0; i < count; i++) {
    out.push_back(in[i]);
  }

  model.surfedges = out;
}

void BspParser::loadTextures(Lump *l) {

  MipTexLump *m;
  MipTex *mt;
  int pixels = 0;

  if (!l->filelen) {
    LOG << "loadTextures: No textures in BSP file.";
    return;
  }

  m = (MipTexLump*) (modelBytes + l->fileofs);
  m->nummiptex = Utility::littleLong(m->nummiptex);

  for (int i = 0; i < m->nummiptex; i++) {
    m->dataofs[i] = Utility::littleLong(m->dataofs[i]);
    if (m->dataofs[i] == -1) {
      continue;
    }

    mt = (MipTex*) ((byte*) m + m->dataofs[i]);
    mt->width = Utility::littleLong(mt->width);
    mt->height = Utility::littleLong(mt->height);

    for (int j = 0; j < MIPLEVELS; j++) {
      mt->offsets[j] = Utility::littleLong(mt->offsets[j]);
      // If offsets are 0 then texture is stored else where and not in the BSP file.
    }

    if ((mt->width & 15) || (mt->height & 15)) {
      LOG << "Texture " << mt->name << " is not 16 aligned";
    }

    Texture tx;
    memcpy(tx.name, mt->name, sizeof(tx.name));
    tx.width = mt->width;
    tx.height = mt->height;

    byte *bytez = (byte*) mt + mt->offsets[0];
    pixels = mt->width * mt->height;

    for (size_t i = 0; i < pixels; i++) {
      tx.buffer.push_back(table[bytez[i]]);
      tx.buffer.push_back(table[bytez[i]] >> 8);
      tx.buffer.push_back(table[bytez[i]] >> 16);
      tx.buffer.push_back(255);
    }

    model.textures.push_back(tx);
  }

}

void BspParser::loadTexInfo(Lump *l) {
  TexInfo *in;

  int count = 0;
  in = (TexInfo*) (modelBytes + l->fileofs);

  if (l->filelen % sizeof(*in)) {
    LOG << "loadTexInfo: funny lump size in " << model.name;
    return;
  }

  count = l->filelen / sizeof(*in);

  for (int i = 0; i < count; i++, in++) {

    ModelTexInfo out;

    for (int j = 0; j < 4; j++) {
      out.vecs[0][j] = Utility::littleFloat(in->vecs[0][j]);
      out.vecs[1][j] = Utility::littleFloat(in->vecs[1][j]);
    }

    size_t mipTex = Utility::littleLong(in->miptex);
    out.flags = Utility::littleLong(in->flags);

    if (mipTex < model.textures.size()) {
      out.texture = model.textures.at(mipTex);
    }
//    LOG << out.texture.name << " TexInfo s " << out.vecs[0][0] << " " << out.vecs[0][1] << " " << out.vecs[0][2] << " " << out.vecs[0][3];
//    LOG << out.texture.name << " TexInfo t " << out.vecs[1][0] << " " << out.vecs[1][1] << " " << out.vecs[1][2] << " " << out.vecs[1][3];

    // Ignoring error handling for missing textures.
    model.texinfos.push_back(out);
  }

}

void BspParser::loadEntities(Lump *l) {
  if (!l->filelen) {
    LOG << "Skipping loading of entities";
    model.entities = nullptr;
    return;
  }

  model.entities = (char*) (modelBytes + l->fileofs);

  std::vector<std::string> strings = Utility::split(std::string(model.entities), '\n');

  BspEntity be;

  for (auto s : strings) {
    if (s == "{") {
      be = BspEntity();
    } else if (s == "}") {
      model.bspEntities.push_back(be);
    } else {
      size_t startKeyIndex = s.find('"', 0) + 1;
      size_t endKeyIndex = s.find('"', startKeyIndex);
      size_t startValueIndex = s.find('"', endKeyIndex + 1) + 1;
      size_t endValueIndex = s.find('"', startValueIndex);

      std::string key = s.substr(startKeyIndex, endKeyIndex - startKeyIndex);
      std::string value = s.substr(startValueIndex, endValueIndex - startValueIndex);
      be.keyValues[key] = value;
    }
  }
}

void BspParser::loadSubmodels(Lump *l) {
  SubModel *ins, *out;

  ins = (SubModel*) (modelBytes + l->fileofs);

  if (l->filelen % sizeof(*ins)) {
    LOG << "loadSubmodels: funny lump size " << model.name;
    return;
  }

  int count = l->filelen / sizeof(*ins);

  if (count > MAX_MODELS) {
    LOG << "loadSubmodels: count > MAX_MODELS";
    return;
  }

  for (size_t i = 0; i < count; i++, ins++) {
    SubModel subModel;
    for (size_t j = 0; j < 3; j++) {
      subModel.mins[j] = Utility::littleFloat(ins->mins[j]) - 1;
      subModel.maxs[j] = Utility::littleFloat(ins->maxs[j]) - 1;
      subModel.origin[j] = Utility::littleFloat(ins->origin[j]);
    }

    std::swap(subModel.mins.z, subModel.mins.y);
    std::swap(subModel.maxs.z, subModel.maxs.y);
    std::swap(subModel.origin.z, subModel.origin.y);

    for (size_t j = 0; j < MAX_MAP_HULLS; j++) {
      subModel.headnode[j] = Utility::littleLong(ins->headnode[j]);
    }

    subModel.visleafs = Utility::littleLong(ins->visleafs);
    subModel.firstface = Utility::littleLong(ins->firstface);
    subModel.numfaces = Utility::littleLong(ins->numfaces);

    model.submodels.push_back(subModel);
  }

}

void BspParser::loadFaces(Lump *l) {
  Face *ins;
  int i, count, surfnum, lofs;
  int planenum, side, texinfon;

  ins = (Face*) (modelBytes + l->fileofs);

  if (l->filelen % sizeof(*ins)) {
    LOG << "loadFaces: funny lump size in " << model.name;
    return;
  }

  count = l->filelen / sizeof(*ins);

  for (surfnum = 0; surfnum < count; surfnum++) {
    Surface out;
    out.firstedge = Utility::littleLong(ins->firstedge);
    out.numedges = Utility::littleLong(ins->numedges);
    planenum = Utility::littleLong(ins->planenum);
    side = Utility::littleLong(ins->side);
    texinfon = Utility::littleLong(ins->texinfo);

    for (i = 0; i < MAXLIGHTMAPS; i++)
      out.styles[i] = ins->styles[i];

    lofs = ins->lightofs;

    ins++;

    out.flags = 0;

    if (side)
      out.flags |= SURF_PLANEBACK;

    out.texinfo = model.texinfos.at(texinfon);
    generateSurface(&out);
    model.surfaces.push_back(out);
  }
}

void BspParser::generateSurface(Surface *fa) {
  int numverts, i, lindex;
// convert edges back to a normal polygon
  numverts = 0;
  float s = 0, t = 0;

  std::vector<ModelEdge> pedges = model.medges;

  for (i = 0; i < fa->numedges; i++) {
    glm::vec3 vec;

    lindex = model.surfedges.at(fa->firstedge + i);
    if (lindex > 0) {
      ModelEdge idx = pedges.at(lindex);
      vec = model.vertexes.at(idx.v[0]);
    } else {
      ModelEdge idx = pedges.at(-lindex);
      vec = model.vertexes.at(idx.v[1]);
    }

    float val = 0;
    float mins[2], maxs[2];
    glm::vec3 vv = glm::vec3(vec.x, vec.z, vec.y);

    mins[0] = mins[1] = 99999999;
    maxs[0] = maxs[1] = -99999999;

    ModelTexInfo *tex = &fa->texinfo;

    for (int j = 0; j < 2; j++) {

      float d = tex->texture.width;

      if (j == 1) {
        d = tex->texture.height;
      }

      val = (vv[0] * tex->vecs[j][0] + vv[1] * tex->vecs[j][1] + vv[2] * tex->vecs[j][2] + tex->vecs[j][3]) / d;

      if (j == 0) {
        s = val;
      } else if (j == 1) {
        t = val;
      }
    }

    glm::vec2 uv(s, t);
    ModelVertex mv { vec, uv };
    fa->polys.verts.push_back(mv);
    numverts++;
  }

  fa->polys.numverts = numverts;
}
