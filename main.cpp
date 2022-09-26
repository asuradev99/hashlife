#include <bitset> 
#include <iostream>
#include <unordered_map>
#include <cmath>
#include <string>
#include <stdexcept>

using cells16 =  unsigned short int;
class Node; 


typedef union {
    Node *ptr;
    cells16 raw; 
} nodeptr; 

#define join_leaf(nw, ne, sw, se) \
    (((nw & 0b1100) | ((ne & 0b1100) >>2)) << 12) | \
    ((((nw & 0b0011) <<2 )| (ne & 0b0011)) << 8) | \
    (((sw & 0b1100) | ((se & 0b1100) >>2)) << 4) | \
    (((sw & 0b0011) <<2 ) | (se & 0b0011))


//bruh asudhashdkajhskdlhlkasjdh
bool life8(cells16 neighbor_bits, int center_bit) {
    int count = 0;
    int center = (neighbor_bits & (1 << center_bit));
    neighbor_bits &= ~(1 << center_bit);
    while (neighbor_bits)
    {
        count += (neighbor_bits & 1);    // check last bit
        neighbor_bits >>= 1;
    }
    return center ? (count == 2 || count == 3) : (count == 3);

};


class Node {
    public: 
        int depth;
        mutable nodeptr nw, ne, sw, se; 
        nodeptr res; 

        const char *name;  

        mutable long int hash; 

        Node(nodeptr, nodeptr, nodeptr, nodeptr, int, const char*);
        Node(cells16, cells16, cells16, cells16, const char*);
        Node(Node*, Node*, Node*, Node*, int, const char*);

        Node* eval();
        Node* clone();

        std::string display(int);
        void display_all(); 
        void setbit(int, int, int); 
        void separate_stream(cells16 stream);
        void zero_extend(int);
        bool operator == (const Node &other) const { 
            if(other.depth != this->depth) {
                 std::cout << "yay you prevented a segfault good job" << std::endl;
                return false; //prevent segmentation fault
            }
            if(depth == 2) {
                return (   this->nw.raw == other.nw.raw \
                        && this->ne.raw == other.ne.raw \
                        && this->sw.raw == other.sw.raw \
                        && this->se.raw == other.se.raw );
            } else {
                std::cout << "Foudn potential match of depth 3" << std::endl;
                //could be a reason for incorrect results if gol has more than one parent gen per child gen
                return (   this->nw.ptr == other.nw.ptr\
                        && this->ne.ptr == other.ne.ptr \
                        && this->sw.ptr == other.sw.ptr\
                        && this->se.ptr == other.se.ptr);
            }
        }
};

namespace std {

  template <>
  struct hash<Node>
  {
    std::size_t operator()(const Node& node) const 
    {
      using std::size_t;
      using std::hash;
      using std::string;

      // Compute individual hash values for first,
      // second and third and combine them using XOR
      // and bit shifting:

      int nw_hash, ne_hash, sw_hash, se_hash;     
      if(node.depth == 2) {
        nw_hash = node.nw.raw; 
        ne_hash = node.ne.raw; 
        sw_hash = node.sw.raw; 
        se_hash = node.se.raw; 
           // node.hash = 4;
      } else {

        nw_hash = node.nw.ptr->hash; 
        ne_hash = node.ne.ptr->hash; 
        sw_hash = node.sw.ptr->hash; 
        se_hash = node.se.ptr->hash; 

               // node.hash = 48;

      }
      node.hash = (hash<int>()(nw_hash) \
                ^ (((hash<int>()(ne_hash) << 1)))\
                ^ (hash<int>()(sw_hash) << 2)\
                ^ (hash<int>()(se_hash) << 3));
        //  node.hash = nw_hash + ne_hash + sw_hash + se_hash; 
        //  std::cout << "hash computation: " << nw_hash << " + " << ne_hash << " + " << sw_hash<< " + " << se_hash << " = " << node.hash << std::endl;

      return node.hash;
    }
  };
}

std::unordered_map<Node, Node*> HASHTABLE;

Node::Node(cells16 nw, cells16 ne, cells16 sw, cells16 se, const char* name) {
    this->nw.raw = nw;  
    this->ne.raw = ne; 
    this->sw.raw = sw; 
    this->se.raw = se; 
    this->depth = 2;
    this->hash = 0;
    this->name = name;
}
Node::Node(Node* nw, Node* ne, Node* sw, Node* se, int depth, const char* name) {
    this->nw.ptr = nw;  
    this->ne.ptr = ne; 
    this->sw.ptr = sw; 
    this->se.ptr = se; 
    this->depth = depth;
    this->hash = 0;
    this->name = name;
}
Node::Node(nodeptr nw, nodeptr ne, nodeptr sw, nodeptr se, int depth, const char* name) {
    this->nw = nw;  
    this->ne = ne; 
    this->sw = sw; 
    this->se = se; 
    this->depth = depth;
    this->hash = 0;
    this->name = name;
}
Node* Node::clone() {
    return new Node(this->nw, this->ne, this->sw, this->se, this->depth, this->name);
}
Node* Node::eval() {
     
    //create temporary squares
    if(this->depth > 2) {
        
        std::cout << "Evaluating as node!" << std::endl;

        std::cout << "Main node test before children hash:" << this->hash << std::endl; 

        this->nw.ptr = this->nw.ptr->eval();
        this->ne.ptr = this->ne.ptr->eval();
        this->sw.ptr = this->sw.ptr->eval();
        this->se.ptr = this->se.ptr->eval();
    } else {
        std::cout << " -- Evaluating as leaf! (name is " << this->name << std::endl;
    }

    auto search = HASHTABLE.find(*this);

    if(search  != HASHTABLE.end()) {
        std::cout << "Found matching node / leaf value at address: ! " << search->second << " name " << search->second->name << std::endl ;
        if(this->depth == 2 ) {
            std::cout << "DEBUG pointer result from hash table: " << search->second->res.raw << std::endl<<std::endl;
            std::cout << "DEBUG pointer depth from hash table: " << search->second->depth << std::endl<<std::endl;

        }
        return search->second;
    } else {
        if (depth > 2) {
            Node* nw_ptr = this->nw.ptr;
            Node* ne_ptr = this->ne.ptr;
            Node* sw_ptr = this->sw.ptr;
            Node* se_ptr = this->se.ptr;
            std::cout << std::endl << "Main node test after children hash: " << this->hash << std::endl; 
            std::cout << "Failed to find hash (as node)" << std::endl;
            int n = HASHTABLE.bucket_count();
          
            std::cout << " -- Evaluating Five new nodes: --" << std::endl;
            Node *nm = new Node(nw_ptr->ne, ne_ptr->nw,  nw_ptr->se, ne_ptr->sw, depth - 1, "nm");
            nm = nm->eval();
            std::cout << "DEBUG: nm res: " << nm->res.raw << std::endl;

            Node *wm = new Node(nw_ptr->sw, nw_ptr->se,  sw_ptr->nw, sw_ptr->ne, depth - 1, "wm");
            wm = wm->eval();


            Node *em = new Node(ne_ptr->sw, ne_ptr->se,  se_ptr->nw, se_ptr->ne, depth - 1, "em");
            em = em->eval();

            Node *sm = new Node(sw_ptr->ne, se_ptr->nw,  sw_ptr->se, se_ptr->sw, depth - 1, "sm");
            sm = sm->eval(); 

            Node *cc = new Node(nw_ptr->se, ne_ptr->sw,  sw_ptr->ne, se_ptr->nw, depth - 1, "cc");
            cc = cc->eval();
            
             std::cout << " -- Evaluating Four inner nodes: --" << std::endl;

            //compute inner squiare
            Node *nw_inner = new Node(nw_ptr->res, nm->res, wm->res, cc->res, depth - 1, "nw_inner");
                nw_inner = nw_inner->eval();
            Node *ne_inner = new Node(nm->res, ne_ptr->res, cc->res, em->res, depth - 1, "ne_inner");
                ne_inner = ne_inner->eval();
            Node *sw_inner = new Node(wm->res, cc->res, sw_ptr->res, sm->res, depth - 1, "sw_inner");
                sw_inner = sw_inner->eval();
            Node *se_inner = new Node(cc->res, em->res, sm->res, se_ptr->res, depth - 1, "se_inner");
                se_inner = se_inner->eval();

            Node *res_ = new Node(nw_inner->res, ne_inner->res, sw_inner->res, se_inner->res, depth - 1, "result cell");

            this->res.ptr = res_;
            HASHTABLE.insert({*this, this});
            std::cout << "insertion completed successfully (node), value is " << this << std::endl << std::endl << std::endl;

            return this;
        } else {
            cells16 stream =  (cells16) join_leaf(this->nw.raw, this->ne.raw, this->sw.raw, this->se.raw);   

            std::cout << "Failed to find hash (as leaf)" << std::endl;
            std::cout << "before hash (leaf)" << this->hash << std::endl;
            std::hash<Node> hashm = std::hash<Node>();
            std::cout << "after hash: " << hashm(*this) << std::endl;
            int _nw = life8(stream & 0b1110111011100000, 10);
            int _ne = life8(stream & 0b0111011101110000, 9);
            int _sw = life8(stream & 0b0000111011101110, 6);
            int _se = life8(stream & 0b0000011101110111, 5); 

            this->res.raw =  ((((((_nw << 1) | _ne) << 1) | _sw) << 1) | _se);
        
            HASHTABLE.insert({*this, this});
            std::cout << "insertion completed successfullly (leaf) with hash " << this->hash << " and result " << this->res.raw << std::endl ;
            std::cout << "My address: " << this << std::endl <<std::endl;
            
            return this;
        }
    }
}
std::string Node::display(int r) {
    if (this->depth == 2) {

        cells16 stream =  (cells16) join_leaf(this->nw.raw, this->ne.raw, this->sw.raw, this->se.raw); 
        std::bitset<16> x(stream);
        std::bitset<4> r1(stream >> 12);
        std::bitset<4> r2((stream << 4) >> 12);
        std::bitset<4> r3((stream << 8) >> 12);
        std::bitset<4> r4((stream << 12) >> 12 );
        if (r == 0) {
            return r1.to_string();
        } else if (r == 1) {
            return r2.to_string();
        } else if (r == 2) {
            return r3.to_string();
        } else if (r == 3) {
           return  r4.to_string();
        } else {
            throw(std::invalid_argument("strong r"));
        }
    } else {
        int n = pow(2, this->depth);
        int sr = r % (n / 2);
        if(r >= n / 2) {
            return (this->sw.ptr->display(sr) + this->se.ptr->display(sr));
        } else {
            return (this->nw.ptr->display(sr) + this->ne.ptr->display(sr));
        }
    }
}
void Node::separate_stream(cells16 stream) {
    this->nw.raw = ((stream & 0b1100000000000000) >> 12) | ((stream & 0b0000110000000000) >> 10); 
    this->ne.raw = ((stream & 0b0011000000000000) >> 10) | ((stream & 0b0000001100000000) >> 8);
    this->sw.raw = ((stream & 0b0000000011000000) >> 4) | ((stream & 0b0000000000001100) >> 2);
    this->se.raw = ((stream & 0b0000000000110000) >> 2) | ((stream & 0b0000000000000011));
}
void Node::setbit(int row, int col, int bit) {
   

    if(this->depth == 2) {
        cells16 stream =  (cells16) join_leaf(this->nw.raw, this->ne.raw, this->sw.raw, this->se.raw); 
        int index = row * 4 + col; 
        int output = bit << (15 - index); 
       
        if(bit == 1) {
            stream = stream | (bit << (15 - index));
            this->separate_stream(stream);

        } else if (bit == 0) {
            stream = stream & (bit << (15 - index));
        } else {
            std::cout << "Error: bit to set must be 0 or 1";
        }
    } else {
        int n = pow(2, this->depth);
        int sr = row % (n / 2);
        int sc = col % (n / 2);
        if(row >= n / 2) {
            if(col >= n / 2) {
                this->se.ptr->setbit(sr, sc, bit);
            } else {
                this->sw.ptr->setbit(sr, sc, bit);
            }
        } else {
            if(col >= n / 2) {
                this->ne.ptr->setbit(sr, sc, bit);
            } else {
                this->nw.ptr->setbit(sr, sc, bit);
            }
        }
    }
}

void Node::display_all() {
    for(int i = 0; i < pow(2, this->depth); i++) {
        std::cout << this->display(i) <<  std::endl;
    }
}

Node* zero_extend(Node node) {
    Node* zero_node = new Node(0, 0, 0, 0, "zero_node");
    int depth = node.depth;
    for (int i = 3; i < depth; i++) {
        zero_node = new Node(zero_node->clone(), zero_node->clone(), zero_node->clone(), zero_node->clone(), i, "zero_node");
    }

    Node* nw = new Node(zero_node->clone(), zero_node->clone(), zero_node->clone(), node.nw.ptr, depth, "nw");
    Node* ne = new Node(zero_node->clone(), zero_node->clone(), node.ne.ptr, zero_node->clone(), depth, "ne");
    Node* sw = new Node(zero_node->clone(), node.sw.ptr, zero_node->clone(), zero_node->clone(), depth, "sw");
    Node* se = new Node(node.se.ptr, zero_node->clone(), zero_node->clone(), zero_node->clone(), depth, "se");

    return new Node(nw, ne, sw, se, depth + 1, "final");

}
// void Node::eval() {

// }
/*
11  11
00  00

11  11
00  00
*/
int main() {
    // Node nw = Node( 0b0000,  0b0000, 0b0000, 0b0010);
    // Node ne = Node( 0b0000, 0b0000, 0b1010, 0b0000);
    // Node sw = Node( 0b0000,  0b0100, 0b0000, 0b0000);
    // Node se = Node(  0b1000, 0b0000, 0b0000, 0b0000);
    // Node test_node =  Node(&nw, &ne, &sw, &se, 3);

    //std::unordered_map<Node, Node*> HASHTABLE;
    // Node nw2 = Node( 0b0000,  0b0000, 0b0000, 0b0010);
    // Node ne2 = Node( 0b0000, 0b0000, 0b1010, 0b0000);
    // Node sw2 = Node( 0b0000,  0b0100, 0b0000, 0b0000);
    // Node se2 = Node(  0b1000, 0b0000, 0b0000, 0b0000);
    // Node test_node2 =  Node(&nw2, &ne2, &sw2, &se2, 3);

    Node nw = Node( 0b1111,  0b1111, 0b1111, 0b1111, "nm");
    Node ne = Node( 0b0000, 0b0000, 0b1010, 0b0000, "ne");
    Node sw = Node( 0b0000,  0b0100, 0b0000, 0b0000, "sw");
    Node se = Node(  0b1000, 0b0000, 0b0000, 0b0000, "se");
    Node* test_node = new Node(&nw, &ne, &sw, &se, 3, "test_node");
   // Node test_noder =  Node(&test_node, &test_node, &test_node, &test_node, 4, "test_noder");
  //  Node test_noderr =  Node(&test_noder, &test_noder, &test_noder, &test_noder, 5, "test_noderr");
   //  Node test_noderrr =  Node(&test_noderr, &test_noderr, &test_noderr, &test_noderr, 6, "test_noderrr");
    // Node nm2 = Node( 0b0000,  0b0000, 0b0000, 0b1111, "nm2");
    // Node ne2 = Node( 0b0000, 0b1000, 0b1010, 0b0000, "ne2");
    // Node sw2 = Node( 0b0000,  0b0100, 0b0000, 0b0000, "sw2");
    // Node se2 = Node(  0b1000, 0b0000, 0b0000, 0b0000, "se2");
    // Node test_node2 =  Node(&nm2, &ne2, &sw2, &se2, 3, "test_node2");

   // test_noderrr.display_all();
   // test_noderrr.eval();
   // test_node.res.ptr->display_all(); 

    // test_node.eval();
    // test_noderrr.res.ptr->display_all();
    
    // std::cout << "2nd test node:" << std::endl;

    // test_node2.eval();

    // test_node2.res.ptr->display();
    Node* test = zero_extend(nw);
    test->setbit(0, 0, 1);
    test->display_all();
    Node* test2 = zero_extend(*test);
    test2->setbit(0, 0, 1);
    test2->display_all();

}


// class Node begin
//    Node NW, NE, SW, SE
//    Node RESULT
// end

// Node.evaluate() begin
//     if this.isLeaf() then
//         this.evaluateCellbyCell()
//     else
//         //Start by recursively evaluating children nodes (See Figure 5)
//         this.NW.evaluate()
//         this.NE.evaluate()
//         this.SW.evaluate()
//         this.SE.evaluate()

//         //Fill in the gaps by creating five more nodes (See Figure 6)
//         Node NM = new Node();
//         nm = nm->eval();
//         std::cout << "DEBUG: nm res: " << nm->res.raw << std::endl;

//         Node *wm = new Node(nw_ptr->sw, nw_ptr->se,  sw_ptr->nw, sw_ptr->ne, depth - 1, "wm");
//         wm = wm->eval();


//         Node *em = new Node(ne_ptr->sw, ne_ptr->se,  se_ptr->nw, se_ptr->ne, depth - 1, "em");
//         em = em->eval();

//         Node *sm = new Node(sw_ptr->ne, se_ptr->nw,  sw_ptr->se, se_ptr->sw, depth - 1, "sm");
//         sm = sm->eval(); 

//         Node *cc = new Node(nw_ptr->se, ne_ptr->sw,  sw_ptr->ne, se_ptr->nw, depth - 1, "cc");
//         cc = cc->eval();
//     end if
// end 
//     //

    
