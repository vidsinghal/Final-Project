#include "mem/cache/replacement_policies/prp.hh"
#include "mem/cache/tags/base.hh"
#include <cassert>
#include <memory>

#include "params/PRP.hh"

#include "base/logging.hh" // For fatal_if
#include "base/random.hh"

//static declaration of cache distribution
float hitProbability[6] = {0.9375,0.875,0.75,0.625,0.5625,0.0625};
int reuseFrequencySum;
float numerator;

PRP::PRP(const Params *p)
    : BaseReplacementPolicy(p)
{
}

void
PRP::invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
const
{
    // Reset reference count
    std::static_pointer_cast<PRPReplData>(replacement_data)->refCount = 0;
    std::static_pointer_cast<PRPReplData>(replacement_data)->lastTouchTick = Tick(0);
}

void
PRP::touch(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    // Update reference count
    std::static_pointer_cast<PRPReplData>(replacement_data)->refCount++;
    std::static_pointer_cast<PRPReplData>(replacement_data)->lastTouchTick = curTick();
}

void
PRP::reset(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    // Set last touch timestamp
    std::static_pointer_cast<PRPReplData>(replacement_data)->lastTouchTick = curTick();
    std::static_pointer_cast<PRPReplData>(replacement_data)->refCount = 1;
}

ReplaceableEntry*
PRP::getVictim(const ReplacementCandidates& candidates) const
{
    // There must be at least one replacement candidate
    assert(candidates.size() > 0);

    // Visit all candidates to find victim
    ReplaceableEntry* victim = candidates[0];
    for(int i=0;i<6;i++) 
        {
            reuseFrequencySum += victim->reuseFrequency[i];
            numerator += (victim->reuseFrequency[i]*hitProbability[i]);
        }
        std::static_pointer_cast<PRPReplData>(
            victim->replacementData)->finalBlkProbability = numerator/reuseFrequencySum;

    for (const auto& candidate : candidates) {
        for(int i=0;i<6;i++) 
        {
            reuseFrequencySum += candidate->reuseFrequency[i];
            numerator += (candidate->reuseFrequency[i]*hitProbability[i]);
        }
        std::static_pointer_cast<PRPReplData>(
            candidate->replacementData)->finalBlkProbability = numerator/reuseFrequencySum;

        if (std::static_pointer_cast<PRPReplData>(candidate->replacementData)->finalBlkProbability < 
                    std::static_pointer_cast<PRPReplData>(victim->replacementData)->finalBlkProbability)
                    {
                        victim = candidate;
                    }
        }

    return victim;
}

std::shared_ptr<ReplacementData>
PRP::instantiateEntry()
{
    return std::shared_ptr<ReplacementData>(new PRPReplData());
}

PRP*
PRPParams::create()
{
    return new PRP(this);
}