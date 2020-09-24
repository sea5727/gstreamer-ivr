#ifndef __MS_HMP_MEMORY_HPP__
#define __MS_HMP_MEMORY_HPP__

#include <unistd.h>
#include <stdio.h> 
#include <stdint.h>
#include <string>
#include <vector>

#define MAX_TOOL 10000

using std::vector;
using std::string;



typedef struct _encoder
{
    string type;
}_ENCODER;

typedef struct _dncoder
{
    string type;
}_DECODER;

typedef struct _rtp
{
    int local_udp_port;
    int remote_udp_port;
    string remote_ip;
    int in_payload_type;
    int out_payload_type;
    int dtmf_out_payload_type;
    int dtmf_in_payload_type;
}_RTP;

typedef struct _events
{
    string type;
    bool enabled;

}_EVENTS;

typedef struct _evd_side
{
    bool enabled;
    string tone_suppression;
    vector<string> events;
}_EVD_SIDE;


typedef struct _evd
{
    _EVD_SIDE decoder_side;
}_EVD;


typedef struct _evg
{
    bool enabled;
}_EVG;

typedef struct  _tool_data {
    string tool_type;
    int backend_tool_id;
    bool input_from_RTP;
    bool output_to_RTP;
    _ENCODER decoder;
    _DECODER encoder;
    _RTP RTP;
    vector<_EVENTS> events;
    _EVD EVD;
    _EVG EVG;
    bool audio_enabled;
    vector<int> audio_dst_tool_ids;
    bool video_enabled;
    vector<int> video_dst_tools_ids;

}_TOOL_DATA;

typedef struct _tool
{
    int         req_id;
    bool        use;
    _TOOL_DATA  data;
}_TOOL;




extern _TOOL	_HMP_TOOLS[MAX_TOOL];




#endif

