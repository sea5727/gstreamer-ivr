import gi
gi.require_version('Gst', '1.0')
from gi.repository import Gst, GstRtspServer, GObject


PIPELINE_DESC = '''
audiomixer name=mix ! opusenc ! webmmux ! filesink location=/home/ysh8361/save/bidirectional.webm 
sync=true audiotestsrc ! audioconvert ! mix. 
audiotestsrc wave=9 ! mix.
'''


class RtspFactory:
    def __init__(self):
        GstRtspServer.RTSPMediaFactory.__init__(self)

    def do_create_element(self, url):
        spec = '''
		filesrc location=test.mp4 ! qtdemux name=demux
		demux.video_0 ! queue ! rtph264pay pt=96 name=pay0
		demux.audio_0 ! queue ! rtpmp4apay pt=97 name=pay1
		demux.subtitle_0 ! queue ! rtpgstpay pt=98 name=pay2
        '''
        return Gst.parse_launch(spec)
class RtspServer:
    def __init__(self):
        '''
        '''
        self.server = GstRtspServer.RTSPServer()
        self.server.set_service('3002')
        f = RtspFactory()
        f.set_shared(True)
        m = self.server.get_mount_points()
        m.add_factory('/test', f)
        self.server.attach(None)

        # self.pipe = Gst.parse_launch(PIPELINE_DESC)
        # self.pipe.set_state(Gst.State.PLAYING)




if __name__ == '__main__':
    '''
    '''
    GObject.threads_init()
    loop = GObject.MainLoop()
    Gst.init(None)
    server = RtspServer()
    loop.run()
    

