#ifndef __MS_MEMORY_HPP__
#define __MS_MEMORY_HPP__

#include "ms_hmp_memory.hpp"
#include "ms_singleton.hpp"
#include <boost/asio.hpp>
#include <vector>
#include <gst/gst.h>

#define _PIPELINE_MEM   _pipeline_containers
#define _DATA_MEM       _data_containers



typedef struct pipe_container
{
    GstElement* pipeline;
}_ST_PIPELINE;

typedef struct data_container
{
    _TOOL tool;
}_ST_DATA_;


extern unsigned int _current_mem_index;
extern _ST_PIPELINE _pipeline_containers[3000];
extern _ST_DATA_ _data_containers[3000];




#endif