import gi
gi.require_version('Gst', '1.0')
from gi.repository import Gst, GLib, GObject


PIPELINE_DESC = '''
filesrc location=/home/ysh8361/all.wav ! wavparse ! audioconvert ! audioresample ! alawenc ! rtppcmapay pt=96 ! udpsink host=192.168.0.4 port=3000
'''

'''
v=0
m=audio 3000 RTP/AVP 96
c=IN IP4 192.168.0.34
a=rtpmap:96 PCMA/8000
'''

class RtpSender:
    def __init__(self):
        '''
        '''
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

