/*
 * Developed by Reem Ashour, Khalifa University of Science and Technology.
 * The provided code implements the labeled annotated octomap
 * for the semantic-aware exploration algorithm.
 * The code is based on the implemetation in OcTreeNode.h and ColorOcTree.h
 * by K.M. Wurm and A. Hornung.
 * Check OcTreeNode.h and ColorOcTree.h for further information
 */

#ifndef OCTOMAP_LABEL_OCTREE_H
#define OCTOMAP_LABEL_OCTREE_H


#include <iostream>
#include <octomap/OcTreeNode.h>
#include <octomap/OccupancyOcTreeBase.h>

namespace octomap {

// forward declaraton for "friend"
class LabelOcTree;

// node definition
class LabelOcTreeNode : public OcTreeNode {
public:
    friend class LabelOcTree; // needs access to node children (inherited)
    
    class Label {
    public:

        enum VoxelType{
            VOXEL_FREE                                  = (uint8_t)0 ,
            VOXEL_UNKNOWN                           	= (uint8_t)1 ,
            VOXEL_OCCUPIED_INTEREST_NOT_VISITED     	= (uint8_t)2 ,
            VOXEL_OCCUPIED_INTEREST_VISITED             = (uint8_t)3 ,
            VOXEL_OCCUPIED_NOT_INTEREST                 = (uint8_t)4
        } ;

        enum VoxelClass{
            VOXEL_FLOOR                              	= (uint8_t)0 ,
            VOXEL_WALL                              	= (uint8_t)1 ,
            VOXEL_TABLE                                 = (uint8_t)2 ,
            VOXEL_CHAIR                                 = (uint8_t)3 ,
            VOXEL_NOT_LABELED                         	= (uint8_t)4 ,
        } ;

        // Empty constructor
        Label() :
            r(255),
            g(255),
            b(255),
            type (VOXEL_UNKNOWN),
            object_class(VOXEL_NOT_LABELED),
            object_ID(-1),
            object_certainty(0) ,
            interest_value (0),
            res_of_obs(0),
            num_of_vis(0) {}

        // constructor
        Label(double _r, double _g, double _b, VoxelType _type,VoxelClass _object_class,uint8_t _object_id,double _object_certainty,double _interest_value , double _res_of_obs, int _num_of_vis) :
            r(_r),
            g(_g),
            b(_b),
            type(_type),
            object_class(_object_class),
            object_ID(_object_id),
            object_certainty(_object_certainty),
            interest_value(_interest_value),
            res_of_obs(_res_of_obs),
            num_of_vis(_num_of_vis) {}


        // constructor to update the interest value only
        // The that is called from the function getAverageChildLabel
        Label(double _r, double _g, double _b, double _interest_value) :
            r(_r),
            g(_g),
            b(_b),
            interest_value(_interest_value){}

        Label(double _r, double _g, double _b, double _interest_value,double _res_of_obs , int _num_of_vis) :
            r(_r),
            g(_g),
            b(_b),
            interest_value(_interest_value),
            res_of_obs(_res_of_obs),
            num_of_vis(_num_of_vis){}

        Label(double _res_of_obs,  int _num_of_vis) :
            res_of_obs(_res_of_obs),
            num_of_vis(_num_of_vis){ }

        // constructor
        Label(double _interest_value) :
            r(255),
            g(255),
            b(255),
            type(VOXEL_UNKNOWN),
            object_class(VOXEL_NOT_LABELED),
            object_ID(-1),
            object_certainty(0),
            interest_value(_interest_value),
            res_of_obs(0),
            num_of_vis(0){}


        inline bool operator== (const Label &other) const {
            return ( r==other.r && g==other.g && b==other.b && type==other.type && object_class==other.object_class && object_ID==other.object_ID && object_certainty ==other.object_certainty &&interest_value==other.interest_value && res_of_obs==other.res_of_obs  && num_of_vis ==other.num_of_vis);
        }
        inline bool operator!= (const Label &other) const {
            return (r!=other.r || g!=other.g || b!=other.b || type!=other.type || object_class!=other.object_class || object_ID!=other.object_ID || object_certainty !=other.object_certainty || interest_value!=other.interest_value || res_of_obs!=other.res_of_obs || num_of_vis!=other.num_of_vis );
        }

        // Voxel information
        double r, g, b;
        VoxelType type ;
        VoxelClass object_class ;
        uint8_t object_ID ;
        double object_certainty ;
        double interest_value ;
        double res_of_obs ; // Resolution of obervation
        int num_of_vis ;

    };

public:
    LabelOcTreeNode() : OcTreeNode() {}

    LabelOcTreeNode(const LabelOcTreeNode& rhs) : OcTreeNode(rhs), label(rhs.label) {}

    bool operator==(const LabelOcTreeNode& rhs) const{
        return (rhs.value == value && rhs.label == label);
    }
    
    void copyData(const LabelOcTreeNode& from){
        OcTreeNode::copyData(from);
        this->label = from.getLabel() ;
    }

    inline Label getLabel() const { return label; }
    inline void  setLabel(Label l) {this->label = l; }
    inline void  setLabel(double _interest_value)
    {
        this->label.interest_value = _interest_value;
    }
    inline void  setLabelCertainty(double _certainty_value)
    {
        this->label.object_certainty = _certainty_value;
    }
    inline void  setLabelNumberofVisits(int _num_of_vis)
    {
        this->label.num_of_vis = _num_of_vis;
    }
    inline void  setLabelResOfObs(double _res_of_obs)
    {
        this->label.res_of_obs =_res_of_obs;
    }


    inline void  setLabel(double _r,double _g,double _b,double _interest_value)
    {
        this->label.r = _r;
        this->label.g = _g;
        this->label.b = _b;
        this->label.interest_value = _interest_value;

    }

    inline void  setLabel(double _res_of_obs, int _num_of_vis)
    {
        this->label.res_of_obs = _res_of_obs;
        this->label.num_of_vis = _num_of_vis ;
    }
    inline void  setLabel(double _r,double _g,double _b,double _interest_value,double _res_of_obs, int _num_of_vis)
    {
        this->label.r = _r;
        this->label.g = _g;
        this->label.b = _b;
        this->label.interest_value = _interest_value;
        this->label.res_of_obs = _res_of_obs ;
        this->label.num_of_vis = _num_of_vis ;
    }

    double getR() {return this->label.r;}
    Label& getLabel() { return label; }

    // has any label been integrated? (pure white is very unlikely...)
    // TODO : ADD a check value to the inner class like timestamp
    //    inline bool isLabelSet() const {
    //        return (  (label.r != 255) || (label.g != 255) || (label.b != 255));
    //    }
    inline bool isLabelSet() const {
        // return (label.interest_value != -1);
        return (label.interest_value != -1 || label.num_of_vis > 0 );

    }
    void updateLabelChildren();


    LabelOcTreeNode::Label getAverageChildLabel() const;

    // file I/O
    std::istream& readData(std::istream &s);
    std::ostream& writeData(std::ostream &s) const;
protected:
    Label label;

};


// tree definition
class LabelOcTree : public OccupancyOcTreeBase <LabelOcTreeNode> {

public:
    /// Default constructor, sets resolution of leafs
    LabelOcTree(double resolution);

    /// virtual constructor: creates a new object of same type
    /// (Covariant return type requires an up-to-date compiler)
    LabelOcTree* create() const {return new LabelOcTree(resolution); }

    std::string getTreeType() const {return "LabelOcTree";}
    
    /**
     * Prunes a node when it is collapsible. This overloaded
     * version only considers the node occupancy for pruning,
     * different labels of child nodes are ignored.
     * @return true if pruning was successful
     */
    virtual bool pruneNode(LabelOcTreeNode* node);
    
    virtual bool isNodeCollapsible(const LabelOcTreeNode* node) const;

    // set node interest value and color at given key or coordinate. Replaces previous color.
    LabelOcTreeNode* setNodeLabel(const OcTreeKey& key,
                                  double r, double g, double b,double interest_val , double res_of_obs , int num_of_vis);


    LabelOcTreeNode* setNodeLabel(float x, float y, float z,
                                  double r, double g, double b,double interest_val ,double res_of_obs , int num_of_vis) {

        OcTreeKey key;
        if (!this->coordToKeyChecked(point3d(x,y,z), key)) return NULL;
        return setNodeLabel(key,r,g,b,interest_val,res_of_obs,num_of_vis);
    }

    // integrate color measurement at given key or coordinate. Average with previous color
    LabelOcTreeNode* averageNodeLabel(const OcTreeKey& key,
                                      double r, double g, double b,double interest_val,double res_of_obs , int num_of_vis);
    
    LabelOcTreeNode* averageNodeLabel(float x, float y, float z,
                                      double r, double g, double b, double interest_val,double res_of_obs , int num_of_vis) {
        OcTreeKey key;
        if (!this->coordToKeyChecked(point3d(x,y,z), key)) return NULL;
        return averageNodeLabel(key,r,g,b,interest_val, res_of_obs , num_of_vis);
    }

    // integrate label measurement at given key or coordinate. Average with previous label
    LabelOcTreeNode* integrateNodeLabel(const OcTreeKey& key,
                                        double r,double g, double b,double interest_val,double res_of_obs , int num_of_vis);
    
    LabelOcTreeNode* integrateNodeLabel(float x, float y,float z,
                                        double r,double g, double b,double interest_val,double res_of_obs , int num_of_vis) {
        OcTreeKey key;
        if (!this->coordToKeyChecked(point3d(x,y,z), key)) return NULL;
        return integrateNodeLabel(key,r,g,b,interest_val, res_of_obs , num_of_vis);
    }

    // update inner nodes, sets color to average child color
    void updateInnerOccupancy();

    // uses gnuplot to plot a RGB histogram in EPS format
    void writeLabelHistogram(std::string filename);
    
protected:
    void updateInnerOccupancyRecurs(LabelOcTreeNode* node, unsigned int depth);

    /**
     * Static member object which ensures that this OcTree's prototype
     * ends up in the classIDMapping only once. You need this as a
     * static member in any derived octree class in order to read .ot
     * files through the AbstractOcTree factory. You should also call
     * ensureLinking() once from the constructor.
     */
    class StaticMemberInitializer{
    public:
        StaticMemberInitializer() {
            LabelOcTree* tree = new LabelOcTree(0.1);
            tree->clearKeyRays();
            AbstractOcTree::registerTreeType(tree);
        }

        /**
         * Dummy function to ensure that MSVC does not drop the
         * StaticMemberInitializer, causing this tree failing to register.
         * Needs to be called from the constructor of this octree.
         */
        void ensureLinking() {};
    };
    /// static member to ensure static initialization (only once)
    static StaticMemberInitializer labelOcTreeMemberInit;

};

//! user friendly output in format (r g b)
std::ostream& operator<<(std::ostream& out, LabelOcTreeNode::Label const& l);

} // end namespace

#endif
