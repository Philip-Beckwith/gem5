/**
 * Copyright (c) 2018 Inria
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Daniel Carvalho
 */

#include "mem/cache/replacement_policies/lru_ipv.hh"

#include <cassert>
#include <memory>
#include <vector>

#include "params/LRUIPV.hh"
#include "debug/LRUDEBUG.hh"

LRUIPV::LRUIPV(const Params *p)
    : BaseReplacementPolicy(p)
{
}

void
LRUIPV::invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
const
{
	DPRINTF(LRUDEBUG, "Inside Invalidate\n");
    //declare Vars
    int currentIndex;
    LRUReplData* dataBlock = std::static_pointer_cast<LRUReplData>(replacement_data);
    
    //Get current Index
    currentIndex = dataBlock->index;
    
    //Move block to next position. Order is important here
    cacheBlocks.erase(cacheBlocks.begin() + currentIndex);
    cacheBlocks.push_back(dataBlock);

    //Updating Index
    for(int i = currentIndex; i< cacheBlocks.size(); i++){
        cacheBlocks[i]->index = i;
    }
    DPRINTF(LRUDEBUG, "Exiting Invalidate\n");
}

void
LRUIPV::touch(const std::shared_ptr<ReplacementData>& replacement_data) const
{
	 DPRINTF(LRUDEBUG, "Inside touch\n");
    //declare Vars
    int currentIndex, targetIndex;
    LRUReplData* dataBlock = std::static_pointer_cast<LRUReplData>(replacement_data);
    
    //Get current/target Index
    currentIndex = dataBlock->index;
    targetIndex = currentIndex<16 ? IPV[currentIndex] : currentIndex; 

    //Move block to next position. Order is important here
    cacheBlocks.erase(cacheBlocks.begin() + currentIndex);
    cacheBlocks.emplace(cacheBlocks.begin()+currentIndex, dataBlock);

    //Updating Index
    for(int i = targetIndex; i<= currentIndex; i++){
        cacheBlocks[i]->index = i;
    }
    DPRINTF(LRUDEBUG, "Exiting touch\n");
}

void
LRUIPV::reset(const std::shared_ptr<ReplacementData>& replacement_data) const
{
	 DPRINTF(LRUDEBUG, "Inside reset\n");
    // Set last touch timestamp
    int targetIndex = 11;
    LRUReplData* dataBlock = std::static_pointer_cast<LRUReplData>(replacement_data);
    
    //Move block to position. 
    cacheBlocks.emplace(cacheBlocks.begin()+currentIndex, dataBlock);

    //Updating Index
    for(int i = targetIndex; i<= cacheBlocks.size(); i++){
        cacheBlocks[i]->index = i;
    }
    DPRINTF(LRUDEBUG, "Exiting reset\n");
}

ReplaceableEntry*
LRUIPV::getVictim(const ReplacementCandidates& candidates) const
{
    DPRINTF(LRUDEBUG, "Inside getVictim\n"); 
    // There must be at least one replacement candidate
    assert(candidates.size() > 0);
    assert(cacheBlocks.size()>15);

    // Visit all candidates to find victim
    ReplaceableEntry* victim = candidates[0];
    for (const auto& candidate : candidates) {
        // Update victim entry if necessary
        if (std::static_pointer_cast<LRUReplData>(
                    candidate->replacementData)->index >
                std::static_pointer_cast<LRUReplData>(
                    victim->replacementData)->index) {
            victim = candidate;
        }
    }

    //Remove from cache blocks
    int index = std::static_pointer_cast<LRUReplData>(
                    victim->replacementData)->index;

    cacheBlocks.erase(cacheBlocks.begin()+index);

    for(int i=index; i< cacheBlocks.size(); i++){
        cacheBlocks[i]->index = i;
    }

    DPRINTF(LRUDEBUG, "Exiting getVictim\n");
    return victim;
}

std::shared_ptr<ReplacementData>
LRUIPV::instantiateEntry()
{
    return std::shared_ptr<ReplacementData>(new LRUReplData());
}

LRUIPV*
LRUIPVParams::create()
{
    return new LRUIPV(this);
}
