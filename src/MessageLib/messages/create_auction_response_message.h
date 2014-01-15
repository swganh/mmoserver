// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include "anh/byte_buffer.h"
#include "base_swg_message.h"

namespace swganh
{
namespace messages
{

struct CreateAuctionResponseMessage : public BaseSwgMessage
{
    uint16_t Opcount() const
    {
        return 3;
    }
    uint32_t Opcode() const
    {
        return 0xE61CC92;
    }

    uint64_t unknown; // default: 0
    uint32_t response_type; // setting this determines what message will display on the client

    /*
    	response_type choices:

    	0: The item has been put up for sale
    	1: You cannot sell that item because the auctioner is invalid
    	2: You cannot sell an invalid item
    	3: Sale failed because the vendor is malfunctioning
    	4: You must specify a valid sale price
    	5: Invalid sale duration
    	6: The item is already for sale
    	7: An unknown error occured while creating the sale
    	8: You cannot sell an item you do not own
    	9: You don't have enough credits to post that Sale. The Commodities Market requires 5 credits.
    	10: An unknown error occured while creating the sale
    	11: An unknown error occured while creating the sale
    	12: An unknown error occured while creating the sale
    	13: You have too many auctions on the commodities market. Please withdraw some sales and pick up any items waiting in the Available Items tab.
    	14: Items can not be sold on the bazaar for more than 6000 credits.
    	15: An unknown error occured while creating the sale
    	16: This vendor is currently deactivated and is not accepting new items to be placed up for sale or offer. If you are the owner of this vendor, please make sure the maintenance fees for this vendor have been paid to reactivate the vendor.
    	17: An unknown error occured while creating the sale
    	18: An unknown error occured while creating the sale
    	19: You cannot sell an item that you are trading.
    	20: You are not allowed to sell an item in a crate.
    	21: You are not allowed to sell that item.
    	22: You can only sell empty containers.
    	23: An unknown error occured while creating the sale
    	24: You have more vendors than you can manage. You must either gain sufficient skill to manage the vendors you have, or remove one or more vendors before being allowed to place items for sale.
    	25: This vendor is at maximum capacity and cannot accept any more items for sale.
    	26: An unknown error occured while creating the sale
    	27: An unknown error occured while creating the sale
    	28: An unknown error occured while creating the sale
    	29: An unknown error occured while creating the sale
    */

    void OnSerialize(swganh::ByteBuffer& buffer) const
    {
        buffer.write(unknown);
        buffer.write(response_type);
    }

    void OnDeserialize(swganh::ByteBuffer& buffer)
    {
        unknown = buffer.read<uint64_t>();
        response_type = buffer.read<uint32_t>();
    }
};

}
} // namespace swganh::messages
