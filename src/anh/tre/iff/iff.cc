// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "iff.h"

#include <stack>

using namespace swganh::tre;

void iff_file::loadIFF(swganh::ByteBuffer& inputstream, std::shared_ptr<VisitorInterface> visitor)
{
    //We use a stack instead of recursion to make things more straightforward to follow.
    std::stack<uint32_t> loader;
    int depth = 1;

    //While the stream is not finished
    while(inputstream.read_position() < inputstream.size())
    {
        //We check to see if our read_position has exited a folder node on the stack
        if(!loader.empty() && inputstream.read_position() == loader.top())
        {
            //It has, so we pop the folder node off the stack
            loader.pop();
            --depth;
        }
        else
        {
            //It hasn't, so we get the name of the next node in the stream
            std::string name = getIFFName_(inputstream);

            //It wasn't an iff, or it had an edge case's name. We continue from here as normal
            if(name.size() == 0 || name == "NULL")
            {
                continue;
            }

            //It was, so we read in the size information
            uint32_t size = inputstream.read<uint32_t>(true);

            //Then we check to see if what it contains is a node or data based the first eight characters
            if(size >= 4 && isFolderNode_(name))
            {
                //We must be a folder.
                loader.push(inputstream.read_position()+size);
                if(visitor != nullptr)
                {
                    visitor->visit_folder(depth++, name, size);
                }
            }
            else
            {
                uint32_t post_data_position_ = inputstream.read_position() + size;
                //If we have an interpreter we wil have it interpret our data
                if(visitor != nullptr)
                {
                    visitor->visit_data(depth, name, size, inputstream);
                }
                inputstream.read_position(post_data_position_);
            }
        }
    }
}

std::string iff_file::getIFFName_(swganh::ByteBuffer& input)
{
    //Lets read ahead 8 bytes. We could peek here, but we'll need to adjust our stream position anyway
    //so lets just read.
    std::uint64_t name_buf = input.read<std::uint64_t>();

    //Get the size of the name we just read from the stream
    unsigned int namesize = getNameSize_((char*)&name_buf, 8);

    if(namesize < 4)
    {
        //The name size is 0. Lets backtrack and hope for the best
        namesize = 0;
        input.read_position_delta(-8);
    }
    else if(namesize < 8)
    {
        //The name size is 4. Lets backtrack 4 bytes to keep other things happy.
        namesize = 4;
        input.read_position_delta(-4);
    }
    else
    {
        //The name size is 8.
        namesize = 8;
    }

    //Return the string we found
    return std::string((char*)&name_buf, namesize);
}

bool iff_file::isNodeNameCharacter_(char c)
{
    //Returns true if the character is within the proper range for an iff node name
    //in regex : [A-Z0-9 ]
    if((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == ' ')
        return true;
    else
        return false;
}

unsigned int iff_file::getNameSize_(char namedata[], unsigned int size)
{
    //Lets get the name size based on this namedata array
    unsigned int i = 0;
    while(i < size)
    {
        if(!isNodeNameCharacter_(namedata[i]))
        {
            break;
        }
        ++i;
    }

    //Return what we think the name size is
    return i;
}

bool iff_file::isFolderNode_(std::string& name)
{
    if(name.substr(0, 4) == "FORM" || name.substr(4, 8) == "FORM")
    {
        return true;
    }
    return false;
}
