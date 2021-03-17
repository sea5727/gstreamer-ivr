from enum import Enum

import gi
gi.require_version('Gst', '1.0')
from gi.repository import Gst, GLib, GObject, GstRtp

# for debug of audio sound, should wave param of audiotestsrc is 0 ( beep sounds )
PIPELINE_DESC = '''
audiotestsrc name=audiotestsrc wave=0 ! audioconvert name=audioconvert ! audioresample ! alawenc ! rtppcmapay name=rtppay pt=96 ! udpsink host=192.168.0.4 port=3000
'''

'''
v=0
m=audio 3000 RTP/AVP 96
c=IN IP4 192.168.0.34
a=rtpmap:96 PCMA/8000
'''

class MODE(Enum):
    SILENCE = 1
    MENT = 2

class Ivr:
    def __init__(self):
        '''
        '''
        self.mode = MODE.SILENCE
        self.idx = 0
        self.filelist = None
        self.pipe = Gst.parse_launch(PIPELINE_DESC)
        self.bus = self.pipe.get_bus()
        loop = GLib.MainLoop()
        self.bus.connect('message::eos', self._on_eos)
        self.bus.connect("message", self._message_received, self.pipe, loop, self)
        self.bus.add_signal_watch()
        self.pipe.set_state(Gst.State.PLAYING)
    
    def _on_eos(self, bus, message):
        '''
        '''
        print('_on_eos mode:', self.mode)
        self.pipe.set_state(Gst.State.READY)

        if self.mode == MODE.SILENCE:
            # nothing
            return
        elif self.mode == MODE.MENT:
            filesrc = self.pipe.get_by_name('filesrc')
            wavparse = self.pipe.get_by_name('wavparse')

            self.pipe.remove(filesrc)
            self.pipe.remove(wavparse)
            filesrc.set_state(Gst.State.NULL)
            wavparse.set_state(Gst.State.NULL)

            audiotestsrc = Gst.ElementFactory.make('audiotestsrc', 'audiotestsrc')
            audiotestsrc.set_property('wave', 0)

            audioconvert = self.pipe.get_by_name('audioconvert')

            self.pipe.add(audiotestsrc)
            audiotestsrc.link(audioconvert)

            rtppay = self.pipe.get_by_name('rtppay')
            rtppay.set_property('timestamp-offset', rtppay.seqnum)

            self.pipe.set_state(Gst.State.PLAYING)
            self.mode = MODE.SILENCE

            print(f'len(self.filelist):{len(self.filelist)}, self.idx:{self.idx}')
            if len(self.filelist) > self.idx :
                GLib.timeout_add(1000, next_file, self) 

    def _message_received(self, bus, message, pipeline, loop, id):
        '''
        '''
        return
        st = message.get_structure()
        print(f'bus:{bus}, st:{st}, pipeline:{pipeline}')
        if message.get_structure():
            print(f'message_received : {message.get_structure().get_name()}') 

    
    def _play_file(self):
        '''
        '''
        self.pipe.set_state(Gst.State.READY)

        if self.mode == MODE.SILENCE:
            audiotestsrc = self.pipe.get_by_name('audiotestsrc')
            self.pipe.remove(audiotestsrc)
            audiotestsrc.set_state(Gst.State.NULL)

            filesrc = Gst.ElementFactory.make('filesrc', 'filesrc')
            filesrc.set_property('location', self.filelist[self.idx])
            self.idx += 1
            waveparse = Gst.ElementFactory.make('wavparse', 'wavparse')
            audioconvert = self.pipe.get_by_name('audioconvert')

            self.pipe.add(filesrc)
            self.pipe.add(waveparse)

            filesrc.link(waveparse)
            waveparse.link(audioconvert)

            rtppay = self.pipe.get_by_name('rtppay')
            rtppay.set_property('timestamp-offset', rtppay.seqnum)

            self.pipe.set_state(Gst.State.PLAYING)
            self.mode = MODE.MENT

        elif self.mode == MODE.MENT:
            filesrc = self.pipe.get_by_name('filesrc')
            filesrc.set_property('location', self.filelist[self.idx])
            self.idx += 1
            rtppay = self.pipe.get_by_name('rtppay')
            rtppay.set_property('timestamp-offset', rtppay.seqnum)
            self.pipe.set_state(Gst.State.PLAYING)
            self.mode = MODE.SILENCE

def next_file(*args):
    print('start next_file')
    ivr = args[0]
    ivr._play_file()
    
def trigger(*args):
    '''
    '''
    print('start ivr')
    ivr = args[0]
    ivr.idx = 0
    ivr.filelist = [
        '/home/ysh8361/1.wav',
        '/home/ysh8361/2.wav',
        '/home/ysh8361/3.wav',
        '/home/ysh8361/4.wav',
        '/home/ysh8361/5.wav',
        '/home/ysh8361/6.wav',
        '/home/ysh8361/7.wav',
        '/home/ysh8361/8.wav',
        '/home/ysh8361/9.wav',
        '/home/ysh8361/10.wav'
    ]
    ivr._play_file()


if __name__ == '__main__':
    '''
    '''
    # print('dir(GstRtp):', dir(GstRtp))
    # print('dir(RTPBasePayload):', dir(GstRtp.RTPBasePayload))
    
    GObject.threads_init()
    Gst.init(None)
    ivr = Ivr()
    loop = GLib.MainLoop()

    _resize_timer_id = GLib.timeout_add(1000, trigger, ivr) 
    loop.run()




