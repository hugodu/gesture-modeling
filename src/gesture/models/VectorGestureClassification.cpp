/*
 *  Copied from the AME Patterns openFrameworks addon.
 *
 *  Author:
 *      Stjepan Rajko
 *      Arts, Media and Engineering Program
 *      Arizona State University
 *
 *  Copyright 2008 Arizona Board of Regents.
 *
 *  This file is part of the AME Patterns openFrameworks addon.
 *
 *  The AME Patterns openFrameworks addon is free software: you can redistribute it
 *  and/or modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation, either version 3 of the License,
 *  or (at your option) any later version.
 *
 *  The AME Patterns openFrameworks addon is distributed in the hope that it will be
 *  useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the AME Patterns openFrameworks addon.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "VectorGestureClassification.h"

#include <ame/patterns/task/filtered_classification.hpp>
#include <ame/patterns/model/chain_skip_hmm.hpp>
#include <ame/observations/training/dynamic_vector.hpp>
#include <ame/observations/training/normal.hpp>

#include <boost/foreach.hpp>

#include <vector>

namespace {

    typedef ame::patterns::model::chain_skip_hmm<ame::observations::dynamic_vector<std::vector<ame::observations::normal> >, long double> gesture_model_type;
    typedef ame::patterns::filtered_classification_task<gesture_model_type, multitouch_filter, ame::patterns::best_match_training> gesture_task_type;
    typedef std::vector<std::vector<double> > recording_type;
    typedef std::vector<recording_type> recordings_type;
}

VectorGestureClassification::VectorGestureClassification()
{
    mClassificationTask = new gesture_task_type;
    mLastRecognition = -1;
}

VectorGestureClassification::~VectorGestureClassification()
{
    delete static_cast<gesture_task_type *>(mClassificationTask);
}

/**
 * For now, a gesture is added with a filter. Meaning 1 model + 1 filter = 1 gesture.
 * We'll deal with multiple filters for a model soon.
 */
void VectorGestureClassification::addGestureWithExamplesAndFilter
(
	const std::vector<std::vector<std::vector<double> > > &examples,
	int num_states,
	multitouch_filter filter
)
{
	addGestureWithExamples(examples, num_states);
    //Add the a filter for the model learnt above. It should be the last model.
    static_cast<gesture_task_type *>(mClassificationTask)
            ->
				add_filter_for_pattern(filter,
										static_cast<gesture_task_type *>(mClassificationTask) -> models().size() - 1);
}


void VectorGestureClassification::addGestureWithExamples
(
    const std::vector<std::vector<std::vector<double> > > &examples,
    int num_states
)
{
    static_cast<gesture_task_type *>(mClassificationTask)
        ->
            add_pattern_with_examples
            (
                num_states,
                examples,
                ame::observations::training::dynamic_vector
                <
                    std::vector<ame::observations::normal>
                >
                (
                    ame::observations::training::normal(0.1)
                )
            );
}


int VectorGestureClassification::classify(const std::vector<std::vector<double> > &gesture)
{
	//Cache the static cast ?
	//Call reset params for all pairs with this sample
	//It's a hack so each filter is aware of what's to be done with the incoming sample
	for(size_t i = 0; i <  static_cast<gesture_task_type *>(mClassificationTask)->get_num_pairs(); i++)
	{
		multitouch_filter* filter = &(static_cast<gesture_task_type *>(mClassificationTask)->get_filter(i));
		if(filter->accepts(gesture))
			filter->reset_params_for(gesture);
		else //FIXME: This is a temporary hack, this should be done only inside the filtered_classification_task class
		{
			return -1;
		}

	}

    return mLastRecognition =
        static_cast<gesture_task_type *>(mClassificationTask)
            ->
                classify(gesture);
}

int VectorGestureClassification::numGestures() const
{
    return
        static_cast<gesture_task_type *>(mClassificationTask)
            ->
                get_num_pairs();
}

const std::vector<long double> &VectorGestureClassification::probabilities() const
{
    return static_cast<gesture_task_type *>(mClassificationTask)
        ->
            probabilities();
}


