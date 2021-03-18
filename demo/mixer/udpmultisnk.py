import gi
gi.require_version('Gst', '1.0')
from gi.repository import Gst, GLib, GObject

PIPELINE_DESC = '''
audiotestsrc ! audioconvert ! opusenc ! rtpopuspay pt=96 ! multiudpsink clients=192.168.0.3:3000,192.168.0.4:3000
'''

'''
v=0
m=audio 3000 RTP/AVP 96
c=IN IP4 192.168.0.34
a=rtpmap:96 OPUS/48000
a=fmtp:96 media=audio; clock-rate=48000; encoding-name=OPUS; sprop-maxcapturerate=48000; sprop-stereo=0; encoding-params=2;
'''
class RtpSender:
    def __init__(self):
        self.pipe = Gst.parse_launch(PIPELINE_DESC)
        self.pipe.set_state(Gst.State.PLAYING)


if __name__ == '__main__':
    '''
    '''
    GObject.threads_init()
    Gst.init(None)
    sender = RtpSender()
    loop = GLib.MainLoop()
    loop.run()

