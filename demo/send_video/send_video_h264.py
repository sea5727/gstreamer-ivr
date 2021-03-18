import gi
gi.require_version('Gst', '1.0')
from gi.repository import Gst, GLib, GObject
PIPELINE_DESC = '''
videotestsrc ! x264enc tune=zerolatency bitrate=500 speed-preset=superfast ! rtph264pay pt=96 ! udpsink host=192.168.0.4 port=3004
'''

'''
v=0
m=video 3004 RTP/AVP 96
c=IN IP4 192.168.0.34
a=rtpmap:96 H264/90000
a=fmtp:96 packetization-mode=1; profile-level-id=f4000d; sprop-parameter-sets=Z/QADZGWgUH7AWoMAgqAAAADAIAAAB5HihVQ\,aO8xkhk\=; a-framerate=30;
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

