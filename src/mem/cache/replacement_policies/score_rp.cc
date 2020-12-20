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
 */

#include "mem/cache/replacement_policies/score_rp.hh"

#include <cassert>
#include <memory>

#include "base/random.hh"
#include "params/SCORERP.hh"

SCORERP::SCORERP(const Params *p)
    : BaseReplacementPolicy(p),
      SCOREMax(p->SCOREMax),
      SCOREThreshold(p->SCOREThreshold),
      SCOREIncreaseVel(p->SCOREIncreaseVel),
      SCOREDecreaseVel(p->SCOREDecreaseVel)
{
}

void
SCORERP::invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
const
{
    std::static_pointer_cast<SCOREReplData>(replacement_data)->valid = false;
    // Set last touch SCORE
    std::static_pointer_cast<SCOREReplData>(replacement_data)->lastTouchScore = 0;
}

void
SCORERP::touch(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    // Update last touch SCORE
    if (std::static_pointer_cast<SCOREReplData>(replacement_data)->lastTouchScore == SCOREMax) {
    	std::static_pointer_cast<SCOREReplData>(replacement_data)->lastTouchScore = SCOREMax;
    } else {
    	if (std::static_pointer_cast<SCOREReplData>(replacement_data)->lastTouchScore + SCOREIncreaseVel > SCOREMax) {
    		std::static_pointer_cast<SCOREReplData>(replacement_data)->lastTouchScore = SCOREMax;
	} else {
    		std::static_pointer_cast<SCOREReplData>(replacement_data)->lastTouchScore =
    		std::static_pointer_cast<SCOREReplData>(replacement_data)->lastTouchScore + SCOREIncreaseVel;
	}
    }

}

void
SCORERP::touch(const std::shared_ptr<ReplacementData>& replacement_data, const ReplacementCandidates& candidates) const
{
    // Update last touch SCORE
    if (std::static_pointer_cast<SCOREReplData>(replacement_data)->lastTouchScore == SCOREMax) {
    	std::static_pointer_cast<SCOREReplData>(replacement_data)->lastTouchScore = SCOREMax;
    } else {
    	if (std::static_pointer_cast<SCOREReplData>(replacement_data)->lastTouchScore + SCOREIncreaseVel > SCOREMax) {
    		std::static_pointer_cast<SCOREReplData>(replacement_data)->lastTouchScore = SCOREMax;
	} else {
    		std::static_pointer_cast<SCOREReplData>(replacement_data)->lastTouchScore =
    		std::static_pointer_cast<SCOREReplData>(replacement_data)->lastTouchScore + SCOREIncreaseVel;
	}
    }

    for (const auto& candidate : candidates) {
        if ((std::static_pointer_cast<SCOREReplData>(candidate->replacementData)->valid == true) &&
            (std::static_pointer_cast<SCOREReplData>(candidate->replacementData)->lastTouchScore > 0)  &&
            (candidate->replacementData != replacement_data)) {
        	std::static_pointer_cast<SCOREReplData>(candidate->replacementData)->lastTouchScore = 
        	std::static_pointer_cast<SCOREReplData>(candidate->replacementData)->lastTouchScore - SCOREDecreaseVel;
    	}
    }
}

void
SCORERP::reset(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    std::static_pointer_cast<SCOREReplData>(replacement_data)->valid = true;
    // Set last touch SCORE
    std::static_pointer_cast<SCOREReplData>(replacement_data)->lastTouchScore = 0;
}

void
SCORERP::reset(const std::shared_ptr<ReplacementData>& replacement_data,
               const ReplacementCandidates& candidates, uint64_t SCORERPInitialScore) const
{
    std::static_pointer_cast<SCOREReplData>(replacement_data)->valid = true;
    // Set last touch SCORE
    std::static_pointer_cast<SCOREReplData>(replacement_data)->lastTouchScore = SCORERPInitialScore;

    for (const auto& candidate : candidates) {
        if ((std::static_pointer_cast<SCOREReplData>(candidate->replacementData)->valid == true) &&
            (std::static_pointer_cast<SCOREReplData>(candidate->replacementData)->lastTouchScore > 0)  &&
            (candidate->replacementData != replacement_data)) {
        	std::static_pointer_cast<SCOREReplData>(candidate->replacementData)->lastTouchScore = 
        	std::static_pointer_cast<SCOREReplData>(candidate->replacementData)->lastTouchScore - SCOREDecreaseVel;
    	}
    }
}

ReplaceableEntry*
SCORERP::getVictim(const ReplacementCandidates& candidates) const
{
    // There must be at least one replacement candidate
    assert(candidates.size() > 0);

    std::vector<ReplaceableEntry*> CandidatesScoreBelowThreshold;
    std::vector<ReplaceableEntry*> CandidatesScoreAboveThreshold;

    for (const auto& ReplCandidate : candidates) {

        // Stop searching for victims if an invalid entry is found
        if (!std::static_pointer_cast<SCOREReplData>(ReplCandidate->replacementData)->valid) {
            return ReplCandidate;
        }

        if (std::static_pointer_cast<SCOREReplData>(ReplCandidate->replacementData)->lastTouchScore > SCOREThreshold) {
      		CandidatesScoreAboveThreshold.push_back(ReplCandidate);
        }
        else {
        	CandidatesScoreBelowThreshold.push_back(ReplCandidate);
        }
    }

    // Visit all candidates to find victim
    ReplaceableEntry* victim;

    if (CandidatesScoreBelowThreshold.size() > 0) {
    	// Choose one candidate at random
        victim = CandidatesScoreBelowThreshold[random_mt.random<unsigned>(0,CandidatesScoreBelowThreshold.size()-1)];
    } else {
        victim = CandidatesScoreAboveThreshold[0];
    	for (const auto& candidate : CandidatesScoreAboveThreshold) {
    	    if (std::static_pointer_cast<SCOREReplData>(candidate->replacementData)->lastTouchScore <
    	        std::static_pointer_cast<SCOREReplData>(victim->replacementData)->lastTouchScore) {
    	    	victim = candidate;
    	    }
    	}
    }

    return victim;
}

std::shared_ptr<ReplacementData>
SCORERP::instantiateEntry()
{
    return std::shared_ptr<ReplacementData>(new SCOREReplData());
}

SCORERP*
SCORERPParams::create()
{
    return new SCORERP(this);
}
