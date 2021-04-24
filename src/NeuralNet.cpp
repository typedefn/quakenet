
/* 
 * File:   NeuralNet.cpp
 * Author: dervish
 * 
 * Created on March 6, 2021, 11:17 PM
 */

#include "NeuralNet.hpp"
int NeuralNet::numOutputs = 5;

NeuralNet::NeuralNet() {
    bias = -1.0;
    numInputs = 3;
    hiddenLayers = 1;
    neuronsPerHiddenLayer = 8;
    numberOfTotalWeights = 0;

    createNet();
}

NeuralNet::~NeuralNet() {
}

void NeuralNet::createNet() {
    if (hiddenLayers > 0) {
        layers.push_back(NeuronLayer(neuronsPerHiddenLayer, numInputs));
        numberOfTotalWeights += (numInputs * neuronsPerHiddenLayer);
        for (int i = 0; i < hiddenLayers - 1; ++i) {
            layers.push_back(NeuronLayer(neuronsPerHiddenLayer, neuronsPerHiddenLayer));
            numberOfTotalWeights += (neuronsPerHiddenLayer * neuronsPerHiddenLayer);
        }

        layers.push_back(NeuronLayer(numOutputs, neuronsPerHiddenLayer));
        numberOfTotalWeights += (neuronsPerHiddenLayer * numOutputs);
    } else {
        layers.push_back(NeuronLayer(numOutputs, numInputs));
        numberOfTotalWeights += (numInputs * numOutputs);
    }

}

vector<double> NeuralNet::update(vector<double>& inputs) {
    vector<double> outputs;

    int weightCount = 0;

    if (inputs.size() != numInputs) {
        cout << "ANN error input.size != numInputs" << endl;
        return outputs;
    }

    for (int i = 0; i < hiddenLayers + 1; i++) {
        if (i > 0) {
            inputs = outputs;
        }

        outputs.clear();

        for (int j = 0; j < layers.at(i).getNumNeurons(); ++j) {
            double netInput = 0;
            weightCount = 0;
            int numInputs = layers.at(i).getNeurons().at(j).getNumberOfInputs();

            if (numInputs != inputs.size()) {
                cout << "ANN error numInputs != inputs.size()" << endl;
                return outputs;
            }
            
            for (int k = 0; k < numInputs - 1; ++k) {
                netInput += layers.at(i).getNeurons().at(j).getWeights().at(k) * inputs[weightCount++];
            }

            netInput += layers.at(i).getNeurons().at(j).getWeights().at(numInputs - 1) * bias;

            double output = sigmoid(netInput, 1);
            outputs.push_back(output);
        }
    }

    return outputs;

}

double NeuralNet::sigmoid(double activation, double response) {
    double o = 1 / (1 + pow(2.7183, (-activation / response)));
    return o;
}

vector<double> NeuralNet::getWeights() {
    vector<double> weights;

    for (int i = 0; i < hiddenLayers + 1; i++) {
        for (int j = 0; j < layers.at(i).getNumNeurons(); ++j) {
            int numInputs = layers.at(i).getNeurons().at(j).getNumberOfInputs();

            for (int k = 0; k < numInputs - 1; ++k) {
                double w = layers.at(i).getNeurons().at(j).getWeights().at(k);
                weights.push_back(w);
            }

            double w = layers.at(i).getNeurons().at(j).getWeights().at(numInputs - 1);
            weights.push_back(w);
        }
    }
    return weights;
}

void NeuralNet::putWeights(const vector<double>& weights) {
    int sz = weights.size();
    int index = 0;

    if (sz != numberOfTotalWeights) {
        cout << "ANN Cannot continue weights are not equal, expecting " << numberOfTotalWeights << " but got " << sz << endl;
        return;
    }

    for (int i = 0; i < hiddenLayers + 1; i++) {
        for (int j = 0; j < layers.at(i).getNumNeurons(); ++j) {
            int numInputs = layers.at(i).getNeurons().at(j).getNumberOfInputs();
            
            if (numberOfTotalWeights != sz) {
                cout << "ANN error numberOfTotalWeights != sz " << endl;
            }
            
            layers.at(i).getNeurons().at(j).getWeights().clear();
            for (int k = 0; k < numInputs; ++k) {
                double w = weights.at(index);
                layers.at(i).getNeurons().at(j).getWeights().push_back(w);
                index++;
            }
        }
    }
}

vector<int> NeuralNet::calcSplitPoints() {
    vector<int> splitPoints;

    int weightCounter = 0;

    for (int i = 0; i < hiddenLayers + 1; ++i) {
        for (int j = 0; j < layers.at(i).getNumNeurons(); ++j) {
            for (int k = 0; k < layers.at(i).getNeurons().at(j).getNumberOfInputs(); ++k) {
                ++weightCounter;
            }

            splitPoints.push_back(weightCounter - 1);
        }
    }

    return splitPoints;
}