'''
Created on Feb 13, 2012

@author: giulio
'''

FilterString = "({proto} {dir_port} {flags})"


ConfigHard = dict()

ConfigHard['multi-address'] = "MetricMultiAddr"
ConfigHard['multi-homing'] = "MetricMultiHome"
ConfigHard['subnet'] = "MetricSubnet"
ConfigHard['prob'] = "MetricProb"
ConfigHard['printcount'] = "MetricPrintable"

ThreadPoolTemplate = \
"""         <threadpool id="Thread_pool{0}" num_threads="1"><core number="{0}"/></threadpool>
"""

SnifferTemplate= \
"""         <block id="Sniffer{0}" type="PFQSource" sched_type="active" threadpool="Thread_pool{0}">
	            <params>
		            <queues device="eth4">
			            {1}
		            </queues>
	            </params>
         </block>
"""


SnifferPcapTemplate=\
"""
<block id="Sniffer0" type="PcapSource" sched_type="active" threadpool="Thread_pool0"><params><!--<queues device="{source_name}"/>--><source type="{source_type}" name="{source_name}"/>

		{hw_metrics}

		<!-- <bpf_filter expression="" /> --></params></block>
"""

EventFactoryTemplate= \
""" <block id="Factory{id}" type="EventFactory">  <!-- PacketPlus_Factory -->
            <params>
                {Fields}
                <timeouts classes = "{timeout_classes}"/>
                {KeyRules}
            </params>
         </block>
"""

FeatureBlockTemplate=\
"""
    <block id="FeatureBlock{0}" type="FeatureBlock">
        <params>
        </params>
    </block>
"""

FilteringBlockTemplate=\
"""
        <block id="FilteringBlock{0}" type="FilteringBlock">
            <params>
            </params>
        </block>
"""

ExporterBlockTemplate=\
"""
    <block id="ExporterBlock0" type="Exporter">
        <params protocol="{protocol}" ip="{ip}" port="{port}">
        </params>
    </block>
"""

PrimaryKeyTemplate = \
"""
<primary-key name="{name}" proto="{proto_code}" src_port="{src_port}" dst_port="{dst_port}" flags="{flags}" value="{val}" />
"""


ConnectionTemplate = \
"""
<connection src_block="{0}" src_gate="{1}" dst_block="{2}" dst_gate="{3}"/>
"""

TableLibHeader = \
"""
#pragma once

#define DECL(T, id) T* _t##id=0;

#define TCHECK(tname, key, id) \
    (_t##id=(tname)->get( (key) ) )

#define TVALUE(id) \
    *(_t##id)

#define TCONDGET(tname, key, id) \\
    (_t##id=(tname)->get( (key) )) && *(_t##id)

#define TGET(tname, key) \\
    (tname)->get( (key) )

"""

BotstreamLibHeader = \
"""
#include <cstdlib>
#include <type_traits>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <chrono>
#include <sstream>
#include <string>
#include <fstream>

#include "actionlib.hpp"
#include "operators.hpp"
#include "tables.hpp"

#define gt >
#define gt_eq >=
#define lt <
#define lt_eq <=

long timestamp()
{
    namespace sc = std::chrono;

    auto time = sc::system_clock::now(); // get the current time

    auto since_epoch = time.time_since_epoch(); // get the duration since epoch

    auto nanoseconds= sc::duration_cast<sc::nanoseconds>(since_epoch);

    return nanoseconds.count();
}

std::string getClientId()
{
	std::ifstream idfile("id");
	std::string line;
	std::getline(idfile, line);
	return line;
}

"""


BotstreamLibFooter = \
"""
#undef gt
#undef gt_eq
#undef lt
#undef lt_eq
"""

# FEATURE FUNCION TEMPLATE
#
# {0} represents class name
# {1} represents arguments of Operator function
# {2} represents function return value
#

GenericFunctionTemplate = \
"""
extern "C" {2} {0}( double* Input )
{{
    return {1};
}}
"""

# THIS WILL BE BY STATUS

EventTemplate = \
"""
event Events[]
{{
    {0},
    {{ NULL, NULL, {{-1, 0}}, -1 }}

}};
"""

StatusTemplate = \
"""
state_tuple Event_{0}[]
{{
        {1}
}};
"""

FeatureListTemplate = \
"""
feature_function {1}FeatureList[] = 
{{
        {0},
        NULL
}};
"""

ConditionListTemplate = \
"""
condition_tuple {1}ConditionList[] =
{{
       {0}
       {{ NULL, NULL, -1 }}
}};
"""

ActionListTemplate = \
"""
action_function {1}ActionList{2}[] = {{ {0}, NULL }};
"""

