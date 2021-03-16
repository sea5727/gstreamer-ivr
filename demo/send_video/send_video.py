import gi
gi.require_version('Gst', '1.0')
from gi.repository import Gst, GLib, GObject
'''
gst-launch-1.0 filesrc location=/home/ysh8361/example.ts ! queue ! tsparse set-timestamps=true ! rtpmp2tpay ! .send_rtp_sink_0 rtpbin ! udpsink host=192.168.0.1 port=3000
'''

'''
v=0
o=- 0 0 IN IP4 127.0.0.1
s=No Name
c=IN IP4 192.168.0.34
t=0 0
a=tool:GStreamer
m=video 3000 RTP/AVP 33
a=rtpmap:33 MP2T/90000
a=control:mpegts-transport
'''
class RtpSender:
    def __init__(self):
        '''
        '''
        self.pipe = Gst.ElementFactory.make('playbin', 'playbin')
        self.filesrc = Gst.ElementFactory.make('filesrc', 'filesrc')




if __name__ == '__main__':
    '''
    '''
    GObject.threads_init()
    Gst.init(None)
    sender = RtpSender()
    loop = GLib.MainLoop()
    loop.run()

