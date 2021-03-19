import gi
gi.require_version('Gst', '1.0')
from gi.repository import Gst, GLib, GObject


PIPELINE_DESC = '''
audiomixer name=mix ! opusenc ! webmmux ! filesink location=/home/ysh8361/save/bidirectional.webm sync=true audiotestsrc ! audioconvert ! mix. audiotestsrc wave=9 ! mix.
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

