/*
 * BspParser.hpp
 *
 *  Created on: Dec 18, 2021
 *      Author: mastakillah
 */

#ifndef COMMON_SRC_BSPPARSER_HPP_
#define COMMON_SRC_BSPPARSER_HPP_

#include <Common.hpp>
#include <Utility.hpp>

#define MAX_OSPATH 1024
// max length of a quake game pathname
#define MAX_QPATH 64
#define MAX_MAP_HULLS   4
#define MAX_MODELS 512

// automatic ambient sounds
#define NUM_AMBIENTS      4
// lighting
#define MAXLIGHTMAPS  4
#define MAX_DLIGHTS   64
// lumps
#define HEADER_LUMPS  15
// versions
#define BSPVERSION  29
/* RMQ support (2PSB). 32bits instead of shorts for all but bbox sizes (which
 * still use shorts) */
#define BSP2VERSION_2PSB (('B' << 24) | ('S' << 16) | ('P' << 8) | '2')
/* BSP2 support. 32bits instead of shorts for everything (bboxes use floats) */
#define BSP2VERSION_BSP2 (('B' << 0) | ('S' << 8) | ('P' << 16) | ('2'<<24))
// lumps
#define LUMP_ENTITIES 0
#define LUMP_PLANES   1
#define LUMP_TEXTURES 2
#define LUMP_VERTEXES 3
#define LUMP_VISIBILITY 4
#define LUMP_NODES    5
#define LUMP_TEXINFO  6
#define LUMP_FACES    7
#define LUMP_LIGHTING 8
#define LUMP_CLIPNODES  9
#define LUMP_LEAFS    10
#define LUMP_MARKSURFACES 11
#define LUMP_EDGES    12
#define LUMP_SURFEDGES  13
#define LUMP_MODELS   14
#define HEADER_LUMPS  15
// surfaces
#define SURF_PLANEBACK    2
#define SURF_DRAWSKY    4
#define SURF_DRAWSPRITE   8
#define SURF_DRAWTURB   0x10
#define SURF_DRAWTILED    0x20
#define SURF_DRAWBACKGROUND 0x40
#define SURF_UNDERWATER   0x80
#define SURF_NOTEXTURE    0x100
#define SURF_DRAWFENCE    0x200
#define SURF_DRAWLAVA   0x400
#define SURF_DRAWSLIME    0x800
#define SURF_DRAWTELE   0x1000
#define SURF_DRAWWATER    0x2000

enum ModType {
  ModBrush, ModSprite, ModAlias
};

struct Face {
  short planenum;
  short side;

  int firstedge;
  short numedges;
  short texinfo;

// lighting info
  byte styles[MAXLIGHTMAPS];
  int lightofs;
};

struct Vertex {
  float point[3];
};

struct ModelVertex {
  glm::vec3 position;
  glm::vec2 uv;
};

struct Edge {
  // vertex numbers
  unsigned short v[2];
};

struct ModelEdge {
  unsigned int v[2];
  unsigned int cachededgeoffset;
};

struct CacheUser {
  void *data;
};

struct PackFile {
  char name[MAX_QPATH];
  int filepos, filelen;
};

struct Pack {
  char filename[MAX_OSPATH];
  FILE *handle;
  int numfiles;
  std::vector<PackFile> files;
};

struct SubModel {
  glm::vec3 mins;
  glm::vec3 maxs;
  glm::vec3 origin;
  int headnode[MAX_MAP_HULLS];
  int visleafs;
  int firstface, numfaces;
};

struct Plane {
  glm::vec3 normal;
  float dist;
  byte type;
  byte signbits;
  byte pad[2];
};

struct Node {
  int contents;
  int visframe;
  // for bounding box culling
  float minmaxs[6];

  struct Node *parent;

  // node specific
  Plane *plane;
  Node *children[2];

  unsigned int firstsurface;
  unsigned int numsurfaces;
};

struct Leaf {
  int contents;
  int visframe;
  // for bounding box culling
  float minmaxs[6];

  Node *parent;

  byte *compressed_vis;

  int *firstmarksurface;
  int nummarksurfaces;
  // BSP sequence number for leaf's contents
  int key;
  byte ambient_sound_level[NUM_AMBIENTS];
};
#define MIPLEVELS       4

struct Texture {
  char name[32];
  unsigned int id;
  unsigned width;
  unsigned height;
  unsigned offsets[MIPLEVELS];
  std::vector<byte> buffer;
};

struct TexInfo {
  // [s/t][xyz offset]
  float vecs[2][4];
  int miptex;
  int flags;
};

struct MipTexLump {
  int nummiptex;
  int dataofs[4];
};

struct MipTex {
  char name[16];
  unsigned width, height;
  unsigned offsets[MIPLEVELS];
};

struct Polygon {
  int numverts;
//  float verts[4][VERTEXSIZE]; // variable sized (xyz s1t1 s2t2)
  std::vector<ModelVertex> verts;
  std::vector<glm::vec3> normals;
  std::vector<uint32_t> indices;
  std::vector<glm::vec2> uvs;
  std::vector<glm::vec3> vs;
  std::string textureName;
  glm::mat4 modelMatrix;
  std::vector<glm::vec4> diffuse;

  const std::vector<glm::vec3> triangulateVerts() {
    std::vector<glm::vec3> vertices;

    for (uint32_t e : indices) {
      glm::vec3 vert = verts.at(e).position;
      vertices.push_back(vert);
    }

    return vertices;
  }
};

struct ModelTexInfo {
  float vecs[2][4];
  Texture texture;
  int flags;
};

struct Surface {
  int visframe;

  Plane plane;
  int flags;

  // look up in model->surfedges[], negative numbers are backwards edges
  int firstedge;
  int numedges;

  short texturemins[2];
  short extents[2];
  // gl lightmap coordinates
  int light_s, light_t;

  Polygon polys;
  Surface *texturechain;

  ModelTexInfo texinfo;

  // index of this surface's first vert in the VBO
  int vbo_firstvert;

  // lighting info
  int dlightframe;
  unsigned int dlightbits[(MAX_DLIGHTS + 31) >> 5];
  int lightmaptexturenum;
  byte styles[MAXLIGHTMAPS];
  int cached_light[MAXLIGHTMAPS];
  bool cached_dlight;
  byte *samples;
};

struct BspEntity {
  std::map <std::string, std::string> keyValues;
};

struct Model {
  char name[MAX_QPATH];
  // path id of the game directory that this model came from
  unsigned int path_id;
  ModType type;
  int numframes;
  int flags;

  //
  // volume occupied by the model graphics
  //
  glm::vec3 mins, maxs;
  glm::vec3 ymins, ymaxs;
  glm::vec3 rmins, rmaxs;

  //
  // solid volume for clipping
  //
  bool clipbox;
  glm::vec3 clipmins, clipmaxs;

  //
  // brush model
  //
  int firstmodelsurface, nummodelsurfaces;

  int numsubmodels;
  std::vector<SubModel> submodels;

  int numplanes;
  Plane *planes;

  // number of visible leafs, not counting 0
  int numleafs;
  Leaf *leafs;

  std::vector<glm::vec3> vertexes;

  std::vector<uint32_t> edges;
  std::vector<glm::vec2> uvs;
  std::vector<ModelEdge> medges;
  std::vector<BspEntity> bspEntities;

  int numnodes;
  Node *nodes;

  std::vector<ModelTexInfo> texinfos;

  std::vector<Surface> surfaces;

  std::vector<int> surfedges;

  int numclipnodes;

  int nummarksurfaces;
  int *marksurfaces;

  byte *surfvis;

  int numtextures;
  std::vector<Texture> textures;

  byte *visdata;
  byte *lightdata;
  char *entities;

  int bspversion;

  std::vector<glm::vec3> normals;
};

class BspParser {
public:
  BspParser();
  virtual ~BspParser();

  /**
   *  @param mapName Will parse a map by the mapName without the .bsp extension.
   */
  Model loadModel(const std::string &mapName);
  std::vector<glm::vec3> getTriangulatedVertices();
  std::vector<glm::vec3> getTriangulatedNormals();

private:
  Model model;
  byte *modelBytes;
  Pack pak;
  FILE *paletteHandle;
  byte palettes[768];
  unsigned table[256];

  void loadBrushModel();
  void loadVertexData(Lump *l);
  void loadEdges(Lump *l);
  void loadSurfaceEdges(Lump *l);
  void loadTextures(Lump *l);
  void loadTexInfo(Lump *l);
  void loadEntities(Lump *l);
  void loadFaces(Lump *l);
  void loadSubmodels(Lump *l);
  void generateSurface(Surface *fa);
};

#endif /* COMMON_SRC_BSPPARSER_HPP_ */
