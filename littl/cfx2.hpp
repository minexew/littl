/*
    Copyright (c) 2011, 2013 Xeatheran Minexew

    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.
*/

#pragma once

#include <littl/BaseIO.hpp>
#include <littl/List.hpp>

#include <confix2.h>

namespace cfx2
{
    class List;
    static List* newList( cfx2_List* list, bool owner );

    class Node : public li::Iterable<Node, size_t>
    {
        public:
            cfx2_Node* node;

            class Iterator
            {
                Node& node;
                intptr_t i;

                Iterator& operator = ( const Iterator& );

                public:
                    Iterator( Node& node, intptr_t i ) : node( node ), i( i ) {}

                    operator Node() { return node[i]; }
                    Node operator -> () { return node[i]; }
                    Node operator * () { return node[i]; }
                    void operator ++() { i++; }
                    void operator --() { i--; }

                    size_t getIndex() const { return i; }
                    bool isValid() const { return i >= 0 && ( size_t ) i < node.getNumChildren(); }
            };

        public:
            Node( cfx2_Node* node = nullptr ) : node( node )
            {
            }

            void addChild( Node child )
            {
                cfx2_add_child( node, child.node );
            }

            Node clone()
            {
                return cfx2_clone_node( node );
            }

            Node createChild( const char* name, const char* text = nullptr, bool unique = false )
            {
                return Node( cfx2_create_child( node, name, text, unique ? cfx2_unique : cfx2_multiple ) );
            }

            Node findChild( const char* name )
            {
                return Node( cfx2_find_child( node, name ) );
            }

            const char* getAttrib( const char* name )
            {
                const char* value = 0;

                cfx2_get_node_attrib( node, name, &value );

                return value;
            }

            int getAttribInt( const char* name, int def = 0 )
            {
                long value = def;

                cfx2_get_node_attrib_int( node, name, &value );

                return (int) value;
            }

            List* getChildren()
            {
                if ( !node )
                    return 0;

                return newList( node->children, false );
            }

            Iterator getIterator( bool reverse = false )
            {
                return Iterator( *this, reverse ? getNumChildren() - 1 : 0 );
            }

            List* getList( const char* command )
            {
                if ( !node )
                    return 0;

                return newList( cfx2_get_list( node, command ), true );
            }

            const char* getName()
            {
                if ( !node )
                    return 0;

                return node->name;
            }

            size_t getNumChildren()
            {
                return ( node != nullptr && node->children != nullptr ) ? node->children->length : 0;
            }

            const char* getText()
            {
                if ( !node )
                    return 0;

                return node->text;
            }

            bool isNull()
            {
                return node == nullptr;
            }

            void merge( cfx2::Node with )
            {
                cfx2_merge_nodes_2( node, with.node, &node, cfx2_release_left | cfx2_release_right );
            }

            bool query( const char* path, bool allowModifications = true )
            {
                return cfx2_query( node, path, allowModifications, nullptr ) == cfx2_ok;
            }

            const char* queryValue( const char* path, const char* defaultValue = nullptr )
            {
                const char* value = cfx2_query_value( node, path );

                return ( value != nullptr ) ? value : defaultValue;
            }

            void release()
            {
                cfx2_release_node( node );
            }

            void removeChild( Node child )
            {
                cfx2_remove_child( node, child.node );
            }

            void setAttrib( const char* name, const char* value )
            {
                cfx2_set_node_attrib( node, name, value );
            }

            operator bool ()
            {
                return node != 0;
            }

            Node operator [] ( size_t index )
            {
                if ( !node )
                    return 0;

                return cfx2_item( node->children, index, cfx2_Node* );
            }

            virtual size_t iterableGetLength() const
            {// TODO: node = null
                return cfx2_list_length( node->children );
            }

            virtual Node iterableGetItem( size_t index )
            {
                return cfx2_item( node->children, index, cfx2_Node* );
            }
    };

    class List : public li::Iterable<Node, size_t>
    {
        cfx2_List* list;
        bool owner;

        public:
            List( cfx2_List* list, bool owner ) : list( list ), owner( owner )
            {
            }

            ~List()
            {
                if ( owner )
                    cfx2_release_list( list );
            }

            Node get( size_t index )
            {
                return Node( cfx2_item( list, index, cfx2_Node* ) );
            }

            size_t getLength() const
            {
                if ( !list )
                    return 0;

                return list->length;
            }

            virtual size_t iterableGetLength() const
            {
                return getLength();
            }

            virtual Node iterableGetItem( size_t index )
            {
                return get( index );
            }
    };

    static List* newList( cfx2_List* list, bool owner )
    {
        return new List( list, owner );
    }

    class Document : public Node
    {
        int lastError;

        public:
            Document()
            {
                lastError = cfx2_create_node( 0, &node );
            }

            Document( const char* fileName )
            {
                lastError = cfx2_read_file( fileName, &node );
            }

            Document( cfx2_Node* node )
                    : Node( node ), lastError( 0 )
            {
            }

            ~Document()
            {
                cfx2_release_node_2( &node );
            }

            void create( const char* name = nullptr )
            {
                cfx2_release_node_2( &node );
                lastError = cfx2_create_node( name, &node );
            }

            const char* getErrorDesc()
            {
                return cfx2_get_error_desc( lastError );
            }

            bool isOk() const
            {
                return node != nullptr && lastError == cfx2_ok;
            }
        
            void loadFrom(li::SeekableInputStream* input)
            {
                cfx2_release_node_2( &node );

                cfx2_read_from_string(input->readWhole(), &node);
            }

            bool save( const char* fileName )
            {
                return cfx2_save_document( node, fileName ) == cfx2_ok;
            }

            bool save( li::OutputStream* stream )
            {
                char* text = nullptr;
                size_t capacity = 0, used = 0;

                cfx2_write_to_buffer(node, &text, &capacity, &used);

                stream->write(text, used);

                free(text);

                return true;
            }

            Document& operator = ( Node other )
            {
                cfx2_release_node_2( &node );
                node = other.node;

                return *this;
            }
    };
}
