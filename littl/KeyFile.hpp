#pragma once

namespace li
{
    template <int X = 0>
    class KeyFileT: public ReferencedClass
    {   
        SeekableInputStream* input;

        public:
            li_ReferencedClass_override( KeyFileT<X> );

            KeyFileT( SeekableInputStream* input );
            virtual ~KeyFileT();

            bool isOk() const { return input && input->isReadable(); }
            String readKey( const char* keyName );
    };

    #define __li_member( type ) template <int X> type KeyFileT<X>::
    #define __li_member_ template <int X> KeyFileT<X>::

    __li_member_ KeyFileT( SeekableInputStream* input )
        : input( input )
    {
    }

    __li_member_ ~KeyFileT()
    {
        if ( input )
            input->release();
    }

    #define __li_KeyFile_isNewLine( x ) ( (x) == '\n' || (x) == '\r' )
    #define __li_KeyFile_isWhiteSpace( x ) ( (x) == ' ' || (x) == '\t' )

    __li_member( String ) readKey( const char* keyName )
    {
        if ( !isOk() )
            return String();

        input->rewind();
        unsigned inputSize = input->size();

        String key( keyName );
        unsigned keyLength = key.getNumBytes();

        if ( inputSize < keyLength + 1 )
            return String();

        //* The "line" loop
        while ( input->getPos() < inputSize - keyLength - 1 )
        {
            unsigned lineStartsAt = input->getPos(), //* file offset where the current line starts
                currentLength = 0;                   //* current length of the current key name
            char keyChar = 0;

            input->read( &keyChar );
            while ( ( __li_KeyFile_isNewLine( keyChar ) || __li_KeyFile_isWhiteSpace( keyChar ) ) //* if this is a whitespace
                && lineStartsAt < inputSize )                                                   //* and we aren't at the end yet
            {
                input->read( &keyChar, 1 );                               // get next char
                lineStartsAt++;                                          // adjust the offset counter
            }

            if ( lineStartsAt >= inputSize ) //* if we are too far,
                continue;                    //* break the loop

            if ( keyChar == ';' )            //* aww, a comment!
            {
                input->read( &keyChar, 1 );
                //* just continue reading until we find a NewLine ('\n' or '\r')
                while ( !__li_KeyFile_isNewLine( keyChar ) && lineStartsAt + currentLength < inputSize )
                {
                    input->read( &keyChar );
                    currentLength++;
                }

                continue; //* now start the loop again!
            }

            lineStartsAt += currentLength;
            currentLength = 0;

            //* Well, now we should be at the beginning of the actual entry, so let's read it!

            String currentKeyName, currentKeyValue;

            //* read the key name
            while ( !__li_KeyFile_isWhiteSpace( keyChar ) && lineStartsAt + currentLength < inputSize )
            {
                currentKeyName += keyChar;
                currentLength++;
                input->read( &keyChar );
            }

            if ( lineStartsAt + currentLength >= inputSize ) //* aren't we too far??
                continue;                                    //* if so, break the loop

            //* skip the space(s)
            while ( __li_KeyFile_isWhiteSpace( keyChar ) && lineStartsAt + currentLength < inputSize )
            {
                input->read( &keyChar );
                currentLength++;
            }

            //* read the value
            while ( !__li_KeyFile_isNewLine( keyChar ) && lineStartsAt + currentLength < inputSize )
            {
                currentKeyValue += keyChar;
                currentLength++;
                input->read( &keyChar );
            }

            //* NOTE that now, it's OK to get to the absoulute file end - such file is still valid!
            if ( lineStartsAt + currentLength > inputSize )    //* aren't we too far??
                continue;                                      //* break the loop then

            if ( key == currentKeyName )
                return currentKeyValue;
        }

        //* Too bad, not found!
        return String();
    }

    #undef __li_member
    #undef __li_member_

    typedef KeyFileT<> KeyFile;
}
