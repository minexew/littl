
#pragma once

#include <littl/List.hpp>
#include <littl/String.hpp>

namespace li
{
    template<typename Key, typename Value> class Map
    {
        protected:
            struct Pair
            {
                Key key;
                Value value;
            };

            List<Pair> pairs;
            Value empty;

        private:
            Map( const Map& other );

        public:
            Map( const Value& empty = Value() ) : empty( empty )
            {
            }

            Map( Map&& other ) : pairs( ( List<Pair>&& ) other.pairs ), empty( ( Value&& ) other.empty )
            {
                //printf( "moveconst Map\n" );
            }

            void add( const Key& key, const Value& value )
            {
                Pair pair = { key, value };

                pairs.add( pair );
            }

            void clear()
            {
                pairs.clear();
            }

            static Map<String, String> create( int argc, char** argv, Unicode::Char separator = ':' )
            {
                Map<String, String> map;

                for ( int i = 1; i < argc; i++ )
                {
                    String arg = argv[i];

                    int colon = arg.findChar( separator );

                    if ( colon > 0 )
                        map.add( arg.left( colon ), arg.dropLeft( colon + 1 ) );
                }

                return map;
            }

            Value& get( const Key& key )
            {
                iterate2 ( pair, pairs )
                    if ( ( *pair ).key == key )
                        return ( *pair ).value;

                return empty;
            }

            const size_t getSize() const
            {
                return pairs.length();
            }

            bool isEmpty() const
            {
                return pairs.isEmpty();
            }

            bool set( const Key& key, const Value& value )
            {
                iterate2 ( pair, pairs )
                    if ( ( *pair ).key == key )
                    {
                        ( *pair ).value = value;
                        return true;
                    }

                pairs.add( Pair { key, value } );
                return false;
            }
    };
}
