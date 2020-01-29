/* -*-c++-*-
 *  ----------------------------------------------------------------------------
 *
 *       PlantGL: The Plant Graphic Library
 *
 *       Copyright CIRAD/INRIA/INRA
 *
 *       File author(s): F. Boudon (frederic.boudon@cirad.fr) et al. 
 *
 *  ----------------------------------------------------------------------------
 *
 *   This software is governed by the CeCILL-C license under French law and
 *   abiding by the rules of distribution of free software.  You can  use, 
 *   modify and/ or redistribute the software under the terms of the CeCILL-C
 *   license as circulated by CEA, CNRS and INRIA at the following URL
 *   "http://www.cecill.info". 
 *
 *   As a counterpart to the access to the source code and  rights to copy,
 *   modify and redistribute granted by the license, users are provided only
 *   with a limited warranty  and the software's author,  the holder of the
 *   economic rights,  and the successive licensors  have only  limited
 *   liability. 
 *       
 *   In this respect, the user's attention is drawn to the risks associated
 *   with loading,  using,  modifying and/or developing or reproducing the
 *   software by the user in light of its specific status of free software,
 *   that may mean  that it is complicated to manipulate,  and  that  also
 *   therefore means  that it is reserved for developers  and  experienced
 *   professionals having in-depth computer knowledge. Users are therefore
 *   encouraged to load and test the software's suitability as regards their
 *   requirements in conditions enabling the security of their systems and/or 
 *   data to be ensured and,  more generally, to use and operate it in the 
 *   same conditions as regards security. 
 *
 *   The fact that you are presently reading this means that you have had
 *   knowledge of the CeCILL-C license and that you accept its terms.
 *
 *  ----------------------------------------------------------------------------
 */

/*! \file indexarray_iterator.h
    \brief custom iterator for indexarray.
*/

/* ----------------------------------------------------------------------- */


#ifndef __indexarray_iterator_h__
#define __indexarray_iterator_h__

#include "indexarray.h"
#include <stack>

PGL_BEGIN_NAMESPACE

/* ----------------------------------------------------------------------- */

/**
    \class PreOrderConstIterator
    \brief IndexArray is supposed to represent parent-children relationship of a tree graph.
    The iterator iterate through all nodes in a pre-order
*/
template<class ListOfNodeList, class ListOfNodeListPtr = const ListOfNodeList*>
class NodeListConstIterator  {
public:
        typedef typename ListOfNodeList::element_type NodeList;
        typedef typename NodeList::const_iterator NodeIterator;

        typedef typename NodeList::element_type value_t ;
        typedef const value_t * pointer_t;

protected:
        // A TreeGraph representation as a root id and list of children for each node.
        value_t __root;
        ListOfNodeListPtr __children;
        bool __root_given;

        // pointer to the actual node id in the graph
        NodeIterator __current;
        // pointer to end of list of actual node id in the graph
        NodeIterator __current_end;

        // type definition of a pair of pointer and a stack of pairs
        typedef std::pair<NodeIterator,NodeIterator> NodeIteratorPair;
        typedef std::stack<NodeIteratorPair> IteratorStack;

        // stack of  higher value of pointers in the graph hierarchy
        IteratorStack __stack;

        // Pop pointers value from the stack
        inline void pop()
        {
            NodeIteratorPair p = __stack.top();
            __current = p.first;  __current_end = p.second;
            __stack.pop();
        }

        // Push pointers value into the stack
        inline void push()
        {
            __stack.push( NodeIteratorPair(__current, __current_end));
        }

        // advance in the traversal
        virtual void increment() {}

        void multi_increment(int i) {
            for (int j = 0 ; j < i; ++j) increment();
        }

public:

        NodeListConstIterator(ListOfNodeListPtr children, value_t root):
            __children(children),
            __root(root),
            __root_given(false),
            __current((*children)[root].begin()),
            __current_end((*children)[root].end()),
            __stack()
            {
            }

        virtual ~NodeListConstIterator() {
        }

        // Check if we are at the end of the iteration
        inline bool atEnd() const
        {
            return __root_given &&  __atEnd();
        }

        // value access operator for iterator
        inline value_t operator*() const
        {
            return *__current;
        }

        // return pointer to node
        inline pointer_t operator->() const
        {
            return __current.operator->();
        }

                // compare two iterators
        inline bool operator==(NodeListConstIterator other) {
            return __children == other.__children && __current == other.__current;
        }

        // compare two iterators
        inline bool operator!=(NodeListConstIterator other) {
            return __children != other.__children || __current != other.__current;
        }

protected:
        // Check if we are at the end of the recursion
        inline bool __atEnd() const
        {
            return __current == __current_end && __stack.empty();
        }


};

template<class ListOfNodeList, class ListOfNodeListPtr = const ListOfNodeList*>
class PreOrderConstIterator  : public NodeListConstIterator<ListOfNodeList,ListOfNodeListPtr> {
public:
        typedef NodeListConstIterator<ListOfNodeList,ListOfNodeListPtr>  BaseType;
        typedef typename BaseType::value_t value_t;
        typedef typename BaseType::pointer_t pointer_t;
protected:
        typedef typename BaseType::NodeList NodeList;

protected:
    using BaseType::__root_given;
    using BaseType::__root;
    using BaseType::__current;
    using BaseType::__current_end;
    using BaseType::__children;
    using BaseType::__stack;

        // advance in the traversal
        virtual void increment() {
            if (!__root_given) {
                __root_given = true;
            }
            else {
                value_t currentid = *__current;
                const NodeList& childlist = (*__children)[currentid];
                if (!childlist.empty()){
                    BaseType::push();
                    __current = childlist.begin();
                    __current_end = childlist.end();
                }
                else {
                    if (__current != __current_end) ++__current;
                    while (__current == __current_end && !__stack.empty()) {
                        BaseType::pop();
                        if (__current != __current_end) ++__current;
                    }
                }
            }
        }

public:

        PreOrderConstIterator(ListOfNodeListPtr children, uint32_t root):
            NodeListConstIterator<ListOfNodeList,ListOfNodeListPtr>(children,root)
            {
            }

        virtual ~PreOrderConstIterator() {
        }

        // value access operator for iterator
        inline value_t operator*() const
        {
            if (!__root_given) return __root;
            else return *__current;
        }

        // return pointer to node
        inline pointer_t operator->() const
        {
            if (!__root_given) return &__root;
            else return __current.operator->();
        }

        // pre increment operator for iterator
        inline PreOrderConstIterator& operator++()
        {
            increment();
            return *this;
        }

        // post incremental operator for iterator
        inline PreOrderConstIterator operator++(int i)
        {
            PreOrderConstIterator original = *this;
            BaseType::multi_increment(i);
            return original;
        }


        inline PreOrderConstIterator& operator+=(int i)
        {   // increment by an integer
            BaseType::multi_increment(i);
            return *this;
        }

        inline PreOrderConstIterator operator+(int i) const
        {   // increment by an integer
            PreOrderConstIterator newiter = *this;
            newiter.multi_increment(i);
            return newiter;
        }


};
typedef PreOrderConstIterator<IndexArray,IndexArrayPtr> IndexArrayPreOrderConstIterator;

template<class ListOfNodeList, class ListOfNodeListPtr = const ListOfNodeList*>
class PostOrderConstIterator  : public NodeListConstIterator<ListOfNodeList,ListOfNodeListPtr> {
public:
        typedef NodeListConstIterator<ListOfNodeList,ListOfNodeListPtr>  BaseType;
        typedef typename BaseType::value_t value_t;
        typedef typename BaseType::pointer_t pointer_t;
protected:
        typedef typename BaseType::NodeList NodeList;
protected:
    using BaseType::__root_given;
    using BaseType::__root;
    using BaseType::__current;
    using BaseType::__current_end;
    using BaseType::__children;
    using BaseType::__stack;

        // find first ascendant
        void first_ascendant() {
            if (__current != __current_end) {
                while (true) {
                    const NodeList& childlist = (*__children)[*__current];
                    // if a node has some children, register it in the stack and point to the first children
                    if (!childlist.empty()) {
                        BaseType::push();
                        __current = childlist.begin();
                        __current_end = childlist.end();
                    }
                    else break;
                }
            }
        }

        // advance in the traversal
        virtual void increment() {
            if (__current != __current_end) ++__current;
            // if end of stack, go to ultimate parent : root
            else if (__stack.empty()) __root_given = true;

            if (__current == __current_end) {
                // if node was last of the sibling, go to the parent
                if (!__stack.empty()) BaseType::pop();
             }
            // if node has children, go to its first ascendant
            else first_ascendant();
        }

public:

        PostOrderConstIterator(ListOfNodeListPtr children, uint32_t root):
            NodeListConstIterator<ListOfNodeList,ListOfNodeListPtr>(children,root)
            {
                first_ascendant();
            }

        virtual ~PostOrderConstIterator() {
        }

        // value access operator for iterator
        inline value_t operator*() const
        {
            if (BaseType::__atEnd()) return __root;
            else return *__current;
        }

        // return pointer to node
        inline pointer_t operator->() const
        {
            if (BaseType::__atEnd()) return &__root;
            else return __current.operator->();
        }

         // pre increment operator for iterator
        inline PostOrderConstIterator& operator++()
        {
            increment();
            return *this;
        }

        // post incremental operator for iterator
        inline PostOrderConstIterator operator++(int i)
        {
            PostOrderConstIterator original = *this;
            BaseType::multi_increment(i);
            return original;
        }


        inline PostOrderConstIterator& operator+=(int i)
        {   // increment by an integer
            BaseType::multi_increment(i);
            return *this;
        }

        inline PostOrderConstIterator operator+(int i) const
        {   // increment by an integer
            PostOrderConstIterator newiter = *this;
            newiter.multi_increment(i);
            return newiter;
        }





};
typedef PostOrderConstIterator<IndexArray,IndexArrayPtr> IndexArrayPostOrderConstIterator;

/* ----------------------------------------------------------------------- */

PGL_END_NAMESPACE

#endif
