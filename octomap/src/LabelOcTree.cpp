/*
 * OctoMap - An Efficient Probabilistic 3D Mapping Framework Based on Octrees
 * http://octomap.github.com/
 *
 * Copyright (c) 2009-2013, K.M. Wurm and A. Hornung, University of Freiburg
 * All rights reserved.
 * License: New BSD
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University of Freiburg nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <octomap/LabelOcTree.h>

namespace octomap {


// node implementation  --------------------------------------
std::ostream& LabelOcTreeNode::writeData(std::ostream &s) const {
    s.write((const char*) &value, sizeof(value)); // occupancy
    s.write((const char*) &label, sizeof(Label)); // color
    //s.write((const char*) &label.interest_value, sizeof(label.interest_value)); // occupancy
    //s.write((const char*) &label.type, sizeof(label.type)); // label
    // TODO: only write the type
    return s;
}




std::istream& LabelOcTreeNode::readData(std::istream &s) {
    s.read((char*) &value, sizeof(value)); // occupancy
    s.read((char*) &label, sizeof(Label)); // color

    //s.read((char*) &label.interest_value, sizeof(label.interest_value)); // occupancy
    //s.read((char*) &label.type, sizeof(label.type)); // label
    // TODO: only read the type

    return s;
}

LabelOcTreeNode::Label LabelOcTreeNode::getAverageChildLabel() const {
    double mr = 0;
    double mg = 0;
    double mb = 0;
    double ml = 0;
    int mnov = 0 ;

    int c = 0;

    if (children != NULL){
        for (int i=0; i<8; i++) {
            LabelOcTreeNode* child = static_cast<LabelOcTreeNode*>(children[i]);

            if (child != NULL && child->isLabelSet()) {
                mr += child->getLabel().r;
                mg += child->getLabel().g;
                mb += child->getLabel().b;
                ml += child->getLabel().interest_value ;
                mnov +=child->getLabel().num_of_vis ;

                ++c;
            }
        }
    }

    if (c > 0) {
        mr /= c;
        mg /= c;
        mb /= c;
        ml /= c ;
        mnov /=c ;

        return Label( (double) mr, (double) mg, (double) mb,(double) ml , (int) mnov);
    }
    else { // no child had a label other than white
        return Label();
    }
}


void LabelOcTreeNode::updateLabelChildren() {
    label = getAverageChildLabel();
}


// tree implementation  --------------------------------------
LabelOcTree::LabelOcTree(double resolution)
    : OccupancyOcTreeBase<LabelOcTreeNode>(resolution) {
    labelOcTreeMemberInit.ensureLinking();
};

LabelOcTreeNode* LabelOcTree::setNodeLabel(const OcTreeKey& key,
                                           double r,
                                           double g,
                                           double b,
                                           double interest_value,
                                           int num_of_vis){
    LabelOcTreeNode* n = search (key);
    if (n != 0) {
        n->setLabel(r, g, b,interest_value,num_of_vis);
        // TODO: CHECK WHAT TO RETURN .. It is important
    }
    return n;
}

bool LabelOcTree::pruneNode(LabelOcTreeNode* node) {
    if (!isNodeCollapsible(node))
        return false;

    // set value to children's values (all assumed equal)
    node->copyData(*(getNodeChild(node, 0)));

    if (node->isLabelSet()) // TODO check
        node->setLabel(node->getAverageChildLabel());

    // delete children
    for (unsigned int i=0;i<8;i++) {
        deleteNodeChild(node, i);
    }
    delete[] node->children;
    node->children = NULL;

    return true;
}

bool LabelOcTree::isNodeCollapsible(const LabelOcTreeNode* node) const{
    return false ; //ADDEd from s, Disable it for now

    // all children must exist, must not have children of
    // their own and have the same occupancy probability
    if (!nodeChildExists(node, 0))
        return false;

    const LabelOcTreeNode* firstChild = getNodeChild(node, 0);
    if (nodeHasChildren(firstChild))
        return false;

    for (unsigned int i = 1; i<8; i++) {
        // compare nodes only using their occupancy, ignoring label for pruning
        if (!nodeChildExists(node, i) || nodeHasChildren(getNodeChild(node, i)) || !(getNodeChild(node, i)->getValue() == firstChild->getValue()))
            return false;
    }

    return true;
}

LabelOcTreeNode* LabelOcTree::averageNodeLabel(const OcTreeKey& key,
                                               double r,
                                               double g,
                                               double b,
                                               double interest_value,
                                               int num_of_vis ) {
    LabelOcTreeNode* n = search(key);
    if (n != 0) {
        if (n->isLabelSet()) {
            LabelOcTreeNode::Label prev_label = n->getLabel();
            n->setLabel( (prev_label.r + r)/2, (prev_label.g + g)/2, (prev_label.b + b)/2 , (prev_label.interest_value+interest_value)/2, (prev_label.num_of_vis+num_of_vis)/2);
        }
        else {
            n->setLabel(r, g, b,interest_value,num_of_vis);
        }
    }
    return n;
}

LabelOcTreeNode* LabelOcTree::integrateNodeLabel(const OcTreeKey& key,
                                                 double r,
                                                 double g,
                                                 double b,
                                                 double interest_value,
                                                int num_of_vis) {
    LabelOcTreeNode* n = search (key);
    if (n != 0) {
        if (n->isLabelSet()) {
            LabelOcTreeNode::Label prev_label = n->getLabel();
            double node_prob = n->getOccupancy();

            double new_interest_value = (double) ((double) prev_label.r * node_prob
                                                    +  (double) interest_value * (0.99-node_prob));
            double new_r = (double) ((double) prev_label.r * node_prob
                                       +  (double) r * (0.99-node_prob));
            double new_g = (double) ((double) prev_label.g * node_prob
                                       +  (double) g * (0.99-node_prob));
            double new_b = (double) ((double) prev_label.b * node_prob
                                       +  (double) b * (0.99-node_prob));
            double new_num_of_vis = (double) ((double) prev_label.num_of_vis * node_prob
                                       +  (double) num_of_vis * (0.99-node_prob));
            n->setLabel(new_r, new_g, new_b,new_interest_value,new_num_of_vis);
        }
        else {
            n->setLabel(r, g, b,interest_value,num_of_vis);
        }
    }
    return n;
}


void LabelOcTree::updateInnerOccupancy() {
    this->updateInnerOccupancyRecurs(this->root, 0);
}

void LabelOcTree::updateInnerOccupancyRecurs(LabelOcTreeNode* node, unsigned int depth) {
    // only recurse and update for inner nodes:
    if (nodeHasChildren(node)){
        // return early for last level:
        if (depth < this->tree_depth){
            for (unsigned int i=0; i<8; i++) {
                if (nodeChildExists(node, i)) {
                    updateInnerOccupancyRecurs(getNodeChild(node, i), depth+1);
                }
            }
        }
        node->updateOccupancyChildren();
        node->updateLabelChildren();
    }
}

void LabelOcTree::writeLabelHistogram(std::string filename) {

#ifdef _MSC_VER
    fprintf(stderr, "The label histogram uses gnuplot, this is not supported under windows.\n");
#else
    // build RGB histogram
    std::vector<int> histogram_interest_value (256,0);
    std::vector<int> histogram_r (256,0);
    std::vector<int> histogram_g (256,0);
    std::vector<int> histogram_b (256,0);
    std::vector<int> histogram_num_of_vis (256,0);

    for(LabelOcTree::tree_iterator it = this->begin_tree(),
        end=this->end_tree(); it!= end; ++it) {
        if (!it.isLeaf() || !this->isNodeOccupied(*it)) continue;
        LabelOcTreeNode::Label& c = it->getLabel();
        ++histogram_interest_value[c.interest_value];
        ++histogram_r[c.r];
        ++histogram_g[c.g];
        ++histogram_b[c.b];
        ++histogram_num_of_vis[c.b];

    }
    // plot data
    FILE *gui = popen("gnuplot ", "w");
    fprintf(gui, "set term postscript eps enhanced label\n");
    fprintf(gui, "set output \"%s\"\n", filename.c_str());
    fprintf(gui, "plot [-1:256] ");
    fprintf(gui,"'-' w filledcurve lt 1 lc 1 tit \"r\",");
    fprintf(gui, "'-' w filledcurve lt 1 lc 2 tit \"g\",");
    fprintf(gui, "'-' w filledcurve lt 1 lc 3 tit \"b\",");
    fprintf(gui, "'-' w l lt 1 lc 1 tit \"\",");
    fprintf(gui, "'-' w l lt 1 lc 2 tit \"\",");
    fprintf(gui, "'-' w l lt 1 lc 3 tit \"\"\n");

    for (int i=0; i<256; ++i) fprintf(gui,"%d %d\n", i, histogram_interest_value[i]);
    fprintf(gui,"0 0\n"); fprintf(gui, "e\n");
    for (int i=0; i<256; ++i) fprintf(gui,"%d %d\n", i, histogram_r[i]);
    fprintf(gui,"0 0\n"); fprintf(gui, "e\n");
    for (int i=0; i<256; ++i) fprintf(gui,"%d %d\n", i, histogram_g[i]);
    fprintf(gui,"0 0\n"); fprintf(gui, "e\n");
    for (int i=0; i<256; ++i) fprintf(gui,"%d %d\n", i, histogram_b[i]);
    fprintf(gui,"0 0\n"); fprintf(gui, "e\n");
        for (int i=0; i<256; ++i) fprintf(gui,"%d %d\n", i, histogram_num_of_vis[i]);
    fprintf(gui,"0 0\n"); fprintf(gui, "e\n");

    for (int i=0; i<256; ++i) fprintf(gui,"%d %d\n", i, histogram_interest_value[i]);
    fprintf(gui, "e\n");
    for (int i=0; i<256; ++i) fprintf(gui,"%d %d\n", i, histogram_r[i]);
    fprintf(gui, "e\n");
    for (int i=0; i<256; ++i) fprintf(gui,"%d %d\n", i, histogram_g[i]);
    fprintf(gui, "e\n");
    for (int i=0; i<256; ++i) fprintf(gui,"%d %d\n", i, histogram_b[i]);
    fprintf(gui, "e\n");
    for (int i=0; i<256; ++i) fprintf(gui,"%d %d\n", i, histogram_num_of_vis[i]);
    fprintf(gui, "e\n");
    fflush(gui);
#endif
}

std::ostream& operator<<(std::ostream& out, LabelOcTreeNode::Label const& l) {
    return out << '(' <<(double)l.interest_value << (double)l.r << ' ' << (double)l.g << ' ' << (double)l.b << ' ' << (int)l.num_of_vis<< ')';
}


LabelOcTree::StaticMemberInitializer LabelOcTree::labelOcTreeMemberInit;

} // end namespace

