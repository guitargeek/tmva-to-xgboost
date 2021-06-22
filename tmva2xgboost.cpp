#include <fstream>
#include <iostream>
#include <limits>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

struct SlowTreeNode {
    bool isLeaf = false;
    int depth = -1;
    int index = -1;
    int yes = -1;
    int no = -1;
    int missing = -1;
    int cutIndex = -1;
    double cutValue = 0.0;
    double leafValue = 0.0;
};

using SlowTree = std::vector<SlowTreeNode>;
using SlowForest = std::vector<SlowTree>;

SlowForest load_tmva_xml_to_slowforest(std::string const& xmlpath);

std::string readFile(const char* filename) {
    std::ifstream t(filename);
    std::string str;

    t.seekg(0, std::ios::end);
    str.reserve(t.tellg());
    t.seekg(0, std::ios::beg);

    str.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

    return str;
}

class XMLAttributes {
  public:
    // If we set an attribute that is already set, this will do nothing and return false.
    // Therefore an attribute has repeated and we know a new node has started.
    bool set(std::string const& name, std::string const& value) {
        if (name == "itree")
            return setValue(itree_, std::stoi(value));
        if (name == "boostWeight")
            return setValue(boostWeight_, std::stod(value));
        if (name == "pos")
            return setValue(pos_, value[0]);
        if (name == "depth")
            return setValue(depth_, std::stoi(value));
        if (name == "IVar")
            return setValue(IVar_, std::stoi(value));
        if (name == "Cut")
            return setValue(Cut_, std::stod(value));
        if (name == "res")
            return setValue(res_, std::stod(value));
        if (name == "nType")
            return setValue(nType_, std::stoi(value));
        return true;
    }

    bool hasValue(std::string const& name) {
        if (name == "itree")
            return itree_.has_value();
        if (name == "boostWeight")
            return boostWeight_.has_value();
        if (name == "pos")
            return pos_.has_value();
        if (name == "depth")
            return depth_.has_value();
        if (name == "IVar")
            return IVar_.has_value();
        if (name == "Cut")
            return Cut_.has_value();
        if (name == "res")
            return res_.has_value();
        if (name == "nType")
            return nType_.has_value();
        return false;
    }

    auto const& itree() const { return itree_; };
    auto const& boostWeight() const { return boostWeight_; };
    auto const& pos() const { return pos_; };
    auto const& depth() const { return depth_; };
    auto const& IVar() const { return IVar_; };
    auto const& Cut() const { return Cut_; };
    auto const& res() const { return res_; };
    auto const& nType() const { return nType_; };

    void reset() {
        boostWeight_.reset();
        itree_.reset();
        pos_.reset();
        depth_.reset();
        IVar_.reset();
        Cut_.reset();
        res_.reset();
        nType_.reset();
    }

  private:
    template <class T>
    bool setValue(std::optional<T>& member, T const& value) {
        if (member.has_value()) {
            member = value;
            return false;
        }
        member = value;
        return true;
    }

    // from the tree root node node
    std::optional<double> boostWeight_ = std::nullopt;
    std::optional<int> itree_ = std::nullopt;
    std::optional<char> pos_ = std::nullopt;
    std::optional<int> depth_ = std::nullopt;
    std::optional<int> IVar_ = std::nullopt;
    std::optional<double> Cut_ = std::nullopt;
    std::optional<double> res_ = std::nullopt;
    std::optional<int> nType_ = std::nullopt;
};

struct BDTWithXMLAttributes {
    std::vector<double> boostWeights;
    std::vector<std::vector<XMLAttributes>> nodes;
};

BDTWithXMLAttributes readXMLFile(std::string const& filename);

BDTWithXMLAttributes readXMLFile(std::string const& filename) {
    const std::string str = readFile(filename.c_str());

    std::size_t pos1 = 0;

    std::string name;
    std::string value;

    BDTWithXMLAttributes bdtXmlAttributes;

    std::vector<XMLAttributes>* currentTree = nullptr;

    XMLAttributes* attrs = nullptr;

    while ((pos1 = str.find('=', pos1)) != std::string::npos) {
        auto pos2 = str.rfind(' ', pos1) + 1;

        name = str.substr(pos2, pos1 - pos2);

        pos2 = pos1 + 2;
        pos1 = str.find('"', pos2);

        value = str.substr(pos2, pos1 - pos2);

        if (name == "boostWeight") {
            bdtXmlAttributes.boostWeights.push_back(std::stod(value));
        }

        if (name == "itree") {
            bdtXmlAttributes.nodes.emplace_back();
            currentTree = &bdtXmlAttributes.nodes.back();
            currentTree->emplace_back();
            attrs = &currentTree->back();
        }

        if (bdtXmlAttributes.nodes.empty())
            continue;

        if (attrs->hasValue(name)) {
            currentTree->emplace_back();
            attrs = &currentTree->back();
        }

        attrs->set(name, value);
    }

    if (bdtXmlAttributes.nodes.size() != bdtXmlAttributes.boostWeights.size()) {
        throw std::runtime_error("nodes size and boostWeights size don't match");
    }

    return bdtXmlAttributes;
}

namespace {

    std::vector<SlowTreeNode> getSlowTreeNodes(std::vector<XMLAttributes> const& nodes) {
        std::vector<SlowTreeNode> xgbNodes(nodes.size());

        int xgbIndex = 0;
        for (int depth = 0; xgbIndex != nodes.size(); ++depth) {
            int iNode = 0;
            for (auto const& node : nodes) {
                if (node.depth() == depth) {
                    xgbNodes[iNode].index = xgbIndex;
                    ++xgbIndex;
                }
                ++iNode;
            }
        }

        int iNode = 0;
        for (auto const& node : nodes) {
            auto& xgbNode = xgbNodes[iNode];
            xgbNode.isLeaf = *node.nType() != 0;
            xgbNode.depth = *node.depth();
            xgbNode.cutIndex = *node.IVar();
            xgbNode.cutValue = *node.Cut();
            xgbNode.leafValue = *node.res();
            if (!xgbNode.isLeaf) {
                xgbNode.yes = xgbNodes[iNode + 1].index;
                xgbNode.no = xgbNode.yes + 1;
                xgbNode.missing = xgbNode.yes;
            }
            ++iNode;
        }

        return xgbNodes;
    }

}  // namespace

SlowForest load_tmva_xml_to_slowforest(std::string const& xmlpath) {
    BDTWithXMLAttributes tmvaXML = readXMLFile(xmlpath);
    std::vector<std::vector<SlowTreeNode>> xgboostForest;
    for (auto const& tree : tmvaXML.nodes) {
        xgboostForest.push_back(getSlowTreeNodes(tree));
    }
    return xgboostForest;
}

namespace {

    template <class T>
    void printArray(std::vector<T> const& arr, std::string const& name) {
        std::cout << "\"" << name << "\":[";
        if constexpr (std::is_same<T, double>::value) {
            std::cout << std::scientific;
        }
        int i = 0;
        for (auto const& x : arr) {
            if (i != 0)
                std::cout << ",";
            if constexpr (std::is_same<T, bool>::value) {
                std::cout << (x ? "true" : "false");
            } else {
                std::cout << x;
            }
            ++i;
        }
        std::cout << "]," << std::endl;
        if constexpr (std::is_same<T, double>::value) {
            std::cout.unsetf(std::ios_base::scientific);
        }
    }

    void printZerosFloat(int n, std::string const& name) {
        std::cout << "\"" << name << "\":[";
        for (int i = 0; i < n; ++i) {
            if (i != 0)
                std::cout << ",";
            std::cout << "0.0";
        }
        std::cout << "]," << std::endl;
    }

    void printZerosInt(int n, std::string const& name) {
        std::cout << "\"" << name << "\":[";
        for (int i = 0; i < n; ++i) {
            if (i != 0)
                std::cout << ",";
            std::cout << "0";
        }
        std::cout << "]," << std::endl;
    }
}  // namespace

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "Please pass a TMVA XML weight file and the number of features." << std::endl;
        return 1;
    }

    const int nFeatures = std::stoi(argv[2]);
    const float base_score = 0.0;

    SlowForest forest = load_tmva_xml_to_slowforest(argv[1]);

    std::cout << "{\"learner\":{\"attributes\":{},\"feature_names\":[],\"feature_types\":[],\"gradient_booster\":{"
                 "\"model\":{\"gbtree_model_param\":{\"num_trees\":\""
              << forest.size() << "\",\"size_leaf_vector\":\"0\"}," << std::endl;
    printZerosInt(forest.size(), "tree_info");
    std::cout << "\"trees\":" << std::endl;

    int iTree = 0;

    std::cout << "[" << std::endl;
    for (auto const& tree : forest) {
        auto nNodes = tree.size();

        std::vector<int> nodesMap;
        nodesMap.resize(nNodes);

        {
            int i = 0;
            for (auto const& node : tree) {
                nodesMap[node.index] = i;
                ++i;
            }
        }

        // this is for the json structure
        std::vector<bool> default_left(nNodes);
        std::vector<int> left_children(nNodes);
        std::vector<int> parents(nNodes);
        std::vector<int> right_children(nNodes);
        std::vector<double> split_conditions(nNodes);
        std::vector<int> split_indices(nNodes);

        parents[0] = std::numeric_limits<int>::max();

        for (int i = 0; i < tree.size(); ++i) {
            auto const& node = tree[nodesMap[i]];

            default_left[i] = !node.isLeaf;
            left_children[i] = node.yes;
            if (node.yes != -1) {
                parents[node.yes] = i;
                parents[node.no] = i;
            }
            right_children[i] = node.no;
            split_conditions[i] = node.isLeaf ? node.leafValue : node.cutValue;
            split_indices[i] = std::max(node.cutIndex, 0);
        }

        std::cout << "{" << std::endl;
        printZerosFloat(nNodes, "base_weights");
        std::cout << "\"categories\":[],\"categories_nodes\":[],\"categories_segments\":[],\"categories_sizes\":[],"
                  << std::endl;
        printArray(default_left, "default_left");
        std::cout << "\"id\":" << iTree << "," << std::endl;
        printArray(left_children, "left_children");
        printZerosFloat(nNodes, "loss_changes");
        printArray(parents, "parents");
        printArray(right_children, "right_children");
        printArray(split_conditions, "split_conditions");
        printArray(split_indices, "split_indices");
        printZerosInt(nNodes, "split_type");
        printZerosFloat(nNodes, "sum_hessian");
        std::cout << "\"tree_param\":{\"num_deleted\":\"0\",\"num_feature\":\"" << nFeatures << "\",\"num_nodes\":\""
                  << nNodes << "\",\"size_leaf_vector\":\"0\"}" << std::endl;
        if (iTree != forest.size() - 1) {
            std::cout << "}," << std::endl;
        } else {
            std::cout << "}" << std::endl;
        }

        ++iTree;
    }
    std::cout << "]" << std::endl;
    std::cout << "},\"name\":\"gbtree\"},\"learner_model_param\":{\"base_score\":\""
              << base_score
              << "\",\"num_class\":\"0\",\"num_feature\":\""
              << nFeatures
              << "\"},\"objective\":{\"name\":\"binary:logitraw\",\"reg_loss_param\":{}}},\"version\":[1,4,2]}"
              << std::endl;

    return 0;
}
