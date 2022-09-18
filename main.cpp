#include <bitset> 
#include <iostream>

using cells16 =  unsigned short int;

typedef union {
    void *ptr;
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
        nodeptr nw, ne, sw, se; 
        nodeptr next, res; 
        Node(void*, void*, void*, void*, int);
        Node(int);
        virtual void eval();
        virtual void display();
};

class Leaf : public Node {
    public: 
        Leaf (cells16 nw, cells16 ne, cells16 sw, cells16 se) : Node(2) {
            this->nw.raw = nw;
            this->ne.raw = ne;
            this->sw.raw = sw;
            this->se.raw = se; 
        }
        virtual void eval() {
            std::cout << "Evaluating as leaf!" << std::endl;

            cells16 stream =  (cells16) join_leaf(nw.raw, ne.raw, sw.raw, se.raw);   
            std::bitset<16> x(stream);
            std::cout << x << std::endl;

            int _nw = life8(stream & 0b1110111011100000, 10);
            int _ne = life8(stream & 0b0111011101110000, 9);
            int _sw = life8(stream & 0b0000111011101110, 6);
            int _se = life8(stream & 0b0000011101110111, 5); 

            this->res.raw =  ((((((_nw << 1) | _ne) << 1) | _sw) << 1) | _se);
        }
        virtual void display() {
            cells16 stream =  (cells16) join_leaf(nw.raw, ne.raw, sw.raw, se.raw);   
            std::bitset<4> r1(stream >> 12);
            std::bitset<4> r2((stream << 4) >> 12);
            std::bitset<4> r3((stream << 8) >> 12);
            std::bitset<4> r4((stream << 12) >> 12 );

            std::cout << std::endl << r1 <<std::endl << r2 << std::endl << r3 << std::endl << r4 << std::endl;
        }
};

Node::Node(void *nw, void *ne, void *sw, void *se, int depth) {
    this->nw.ptr = nw;  
    this->ne.ptr = ne; 
    this->sw.ptr = sw; 
    this->se.ptr = se; 
    this->depth = depth;
}
        
Node::Node(int depth) {
    this->depth = depth;
}

void Node::display() {}
void Node::eval() {
    std::cout << "Evaluating as node!" << std::endl;

    Node* nw_ptr = (Node *)this->nw.ptr;
    Node* ne_ptr = (Node *)this->ne.ptr;
    Node* sw_ptr = (Node *)this->sw.ptr;
    Node* se_ptr = (Node *)this->se.ptr;

    nw_ptr->eval();
    ne_ptr->eval();
    sw_ptr->eval();
    se_ptr->eval();
    
    //create temporary squares
    if(this->depth == 3) {
        Leaf nm = Leaf(nw_ptr->ne.raw, ne_ptr->nw.raw,  nw_ptr->se.raw, ne_ptr->sw.raw);
        nm.eval();

        Leaf wm = Leaf(nw_ptr->sw.raw, nw_ptr->se.raw,  sw_ptr->nw.raw, sw_ptr->ne.raw);
        wm.eval();

        Leaf em = Leaf(ne_ptr->sw.raw, ne_ptr->se.raw,  se_ptr->nw.raw, se_ptr->ne.raw);
        em.eval();

        Leaf sm = Leaf(sw_ptr->ne.raw, se_ptr->nw.raw,  sw_ptr->se.raw, se_ptr->sw.raw);
        sm.eval(); 

        Leaf cc = Leaf(nw_ptr->se.raw, ne_ptr->sw.raw,  sw_ptr->ne.raw, se_ptr->nw.raw);
        cc.eval();
        
        //compute inner squiare
        Leaf nw_inner = Leaf(nw_ptr->res.raw, nm.res.raw, wm.res.raw, cc.res.raw);
            nw_inner.eval();
        Leaf ne_inner = Leaf(nm.res.raw, ne_ptr->res.raw, cc.res.raw, em.res.raw);
            ne_inner.eval();
        Leaf sw_inner = Leaf(wm.res.raw, cc.res.raw, sw_ptr->res.raw, sm.res.raw);
            sw_inner.eval();
        Leaf se_inner = Leaf(cc.res.raw, em.res.raw, sm.res.raw, se_ptr->res.raw);
            se_inner.eval();

        Leaf res = Leaf(nw_inner.res.raw, ne_inner.res.raw, sw_inner.res.raw, se_inner.res.raw);
        this->res.ptr = (void *) &res;
        ((Leaf *) this->res.ptr)->display();

    } else {
        // Node nm = Node(nw_ptr->ne, ne_ptr->nw,  nw_ptr->se, ne_ptr->sw);
        // nm.eval();

        // Node wm = Node(nw_ptr->sw, nw_ptr->se , sw_ptr->nw, sw_ptr->ne );
        // wm.eval();

        // Node em = Node(ne_ptr->sw , ne_ptr->se ,  se_ptr->nw , se_ptr->ne );
        // em.eval();

        // Node sm = Node(sw_ptr->ne , se_ptr->nw ,  sw_ptr->se , se_ptr->sw );
        // sm.eval(); 

        // Leaf cc = Leaf(nw_ptr->se , ne_ptr->sw ,  sw_ptr->ne , se_ptr->nw );
        // cc.eval();
        
        // //compute inner squiare
        // Leaf nw_inner = Leaf(nw_ptr->res.raw, nm.res.raw, wm.res.raw, cc.res.raw);
        //     nw_inner.eval();
        // Leaf ne_inner = Leaf(nm.res.raw, ne_ptr->res.raw, cc.res.raw, em.res.raw);
        //     ne_inner.eval();
        // Leaf sw_inner = Leaf(wm.res.raw, cc.res.raw, sw_ptr->res.raw, sm.res.raw);
        //     sw_inner.eval();
        // Leaf se_inner = Leaf(cc.res.raw, em.res.raw, sm.res.raw, se_ptr->res.raw);
        //     se_inner.eval();

        // Leaf res = Leaf(nw_inner.res.raw, ne_inner.res.raw, sw_inner.res.raw, se_inner.res.raw);
        // res.display();
    }
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
    Leaf nw = Leaf( 0b0000,  0b0000, 0b0000, 0b0010);
    Leaf ne = Leaf( 0b0000, 0b0000, 0b1010, 0b0000);
    Leaf sw = Leaf( 0b0000,  0b0100, 0b0000, 0b0000);
    Leaf se = Leaf(  0b1000, 0b0000, 0b0000, 0b0000);
    Node test_node =  Node((void *) &nw, (void *) &ne, (void *) &sw, (void *) &se, 3);
    test_node.eval();
}


class hashtable {
    void *hash_start;
    void *hash_end; 

}