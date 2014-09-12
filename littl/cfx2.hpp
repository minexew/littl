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
#include <littl/String.hpp>

#include <confix2.h>

namespace cfx2
{
    class Node
    {
        public:
            cfx2_Node* node;

            template <typename Node>
            class generic_iterator
            {
                Node& node;
                intptr_t i;

                public:
                    generic_iterator(Node& node, intptr_t i) : node(node), i(i) {}

                    //operator Node() { return node[i]; }
                    Node* operator -> () { return &node[i]; }
                    Node operator * () { return node[i]; }
                    void operator ++() { i++; }
                    //void operator --() { i--; }

                    bool operator != (const generic_iterator<Node>& other) const { return node != other.node || i != other.i; }

                    size_t getIndex() const { return i; }
                    bool isValid() const { return i >= 0 && (size_t)i < node.getNumChildren(); }
            };

            typedef generic_iterator<const Node> const_iterator;

        public:
            Node( cfx2_Node* node = nullptr ) : node( node )
            {
            }

            void addChild( Node child )
            {
                cfx2_add_child( node, child.node );
            }

            cfx2_Node* c_node()
            {
                return node;
            }

            Node clone()
            {
                return cfx2_clone_node( node, cfx2_clone_recursive );
            }

            Node createChild( const char* name, const char* text = nullptr, bool unique = false )
            {
                return Node( cfx2_create_child( node, name, text, unique ? cfx2_unique : cfx2_multiple ) );
            }

            Node findChild( const char* name )
            {
                return Node( cfx2_find_child( node, name ) );
            }

            const char* getAttrib( const char* name ) const
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

            const char* getName()
            {
                if ( !node )
                    return 0;

                return node->name;
            }

            size_t getNumChildren() const
            {
                return ( node != nullptr ) ? cfx2_list_length( node->children ) : 0;
            }

            const char* getText()
            {
                if ( !node )
                    return 0;

                return node->text;
            }

            bool hasAttrib( const char* name )
            {
                return cfx2_find_attrib( node, name ) != nullptr;
            }

            bool isNull()
            {
                return node == nullptr;
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
                cfx2_release_node( &node );
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

            const Node operator [] (size_t index) const
            {
                if ( !node )
                    return 0;

                return cfx2_item( node->children, index, cfx2_Node* );
            }

            bool operator != (const Node& other) const
            {
                return node != other.node;
            }

            // iterators
            const const_iterator begin() const
            {
                return const_iterator( *this, 0 );
            }

            const const_iterator end() const
            {
                return const_iterator( *this, getNumChildren() );
            }
    };

    class Document : public Node
    {
        int lastError;

        public:
            Document()
            {
                lastError = cfx2_create_node( &node );
            }

            Document( const char* fileName )
            {
                load( fileName );
            }

            Document( cfx2_Node* node )
                    : Node( node ), lastError( 0 )
            {
            }

            ~Document()
            {
                cfx2_release_node( &node );
            }

            void create( const char* name = nullptr )
            {
                cfx2_release_node( &node );

                lastError = cfx2_create_node( &node );

                if ( lastError == cfx2_ok )
                    lastError = cfx2_rename_node( node, name );
            }

            const char* getErrorDesc()
            {
                return cfx2_get_error_desc( lastError );
            }

            bool isOk() const
            {
                return node != nullptr && lastError == cfx2_ok;
            }

            bool load( const char* fileName )
            {
                cfx2_release_node( &node );

                lastError = cfx2_read_file( &node, fileName, nullptr );

                return (lastError == cfx2_ok);
            }

            bool loadFrom( li::SeekableInputStream* input )
            {
                cfx2_release_node( &node );

                lastError = cfx2_read_from_string( &node, input->readWhole(), nullptr );

                return (lastError == cfx2_ok);
            }

            bool loadFromString( const char* doc )
            {
                cfx2_release_node( &node );

                if ( doc == nullptr )
                {
                    node = cfx2_new_node( nullptr );
                    return true;
                }

                return cfx2_read_from_string( &node, doc, nullptr ) == cfx2_ok;
            }

            bool save( const char* fileName )
            {
                return cfx2_save_document( node, fileName ) == cfx2_ok;
            }

            bool save( li::OutputStream* stream )
            {
                char* text = nullptr;
                size_t capacity = 0, used = 0;

                cfx2_write_to_buffer( node, &text, &capacity, &used );

                stream->write( text, used );

                free( text );

                return true;
            }

            li::String toString()
            {
                char* text = nullptr;
                size_t capacity = 0, used = 0;

                cfx2_write_to_buffer( node, &text, &capacity, &used );

                li::String ret;
                ret.set( text, used );

                free( text );

                return ret;
            }

            Document& operator = ( Node other )
            {
                cfx2_release_node( &node );
                node = other.node;

                return *this;
            }
    };
}
