import sys
import os
import threading
import socketserver
from code import InteractiveConsole

import dk


class InteractiveServer(socketserver.BaseRequestHandler):
    def handle(self):
        file = self.request.makefile(mode='rw')
        shell = Shell(file)
        try:
            shell.interact()
        except SystemExit:
            pass
        except ConnectionError:
            pass

class Shell(InteractiveConsole):
    def __init__(self, file):
        self.file = sys.stdout = file
        InteractiveConsole.__init__(self)
        return

    def write(self, data):
        self.file.write(data)
        self.file.flush()

    def raw_input(self, prompt=""):
        self.write(prompt)
        return self.file.readline()


class TestApp(dk.App):
    def __init__(self, frame, size, serverPort=0):
        super().__init__()
        self.frame = frame
        self.windowSize = size
        self.serverPort = serverPort

    def onInit(self):
        print('self-initialize.')

        for k, v in self.envPaths.items():
            print('envPaths[{}]: {}'.format(k,v))

        for k, v in self.appInfo.items():
            print('appInfo[{}]: {}'.format(k,v))

        if self.isProxyInstance():
            appExecPath = self.envPaths[dk.app.ENV_PATH_APP_EXECUTABLE]
            resDir = os.path.normpath(os.path.join(appExecPath, '..', '..', 'Resources'))
            if os.path.exists(resDir):
                self.resourceDir = resDir
            else:
                self.resourceDir = self.envPaths[dk.app.ENV_PATH_APP_RESOURCE]
        else:
            self.resourceDir = os.path.abspath(os.path.join( os.path.dirname(__file__), '..', 'Resources' ))
        print('resourceDir: ', self.resourceDir)

        if self.serverPort > 0:
            host = '0.0.0.0'
            port = self.serverPort
            self.server = socketserver.TCPServer((host, port), InteractiveServer)
            print('launching server...')
            threading._start_new_thread(self.server.serve_forever, ())

        displayBounds = self.displayBounds(0)
        contentBounds = self.screenContentBounds(0)
        print('displayBounds: ', displayBounds)
        print('contentBounds: ', contentBounds)

        platform = dk.platform()
        print('platform: ', platform)
        if 'ios' in platform or 'android' in platform:
            self.windowSize = dk.Size(contentBounds.size)
            dk.ui.resource.DEFAULT_UI_SCALE = 2
            self.frame.scaleFactor = 2
        else:
            if self.windowSize.width > contentBounds.width:
                self.windowSize.width = contentBounds.width
            if self.windowSize.height > contentBounds.height:
                self.windowSize.height = contentBounds.height

        print('initialize frame, window, screen')
        window = dk.Window('TestApp', self.windowSize)
        screen = dk.Screen(window, self.frame)
        if not screen:
            print('screen error!?')
            self.terminate(2)
        else:
            self.screen = screen
            self.screen.activeFrameLatency = 0
            self.screen.inactiveFrameLatency = 0
            self.screen.window.activate()
 
    def onExit(self):
        if hasattr(self, 'server'):
            print('shutting down server')
            try:
                self.server.socket.close()
                self.server.shutdown()
            except:
                print("Unexpected error:", sys.exc_info()[0])

        # destroy screen
        self.screen.terminate(True)
        del(self.screen)
        print('cleanup-done')

def runApp(frame, size=dk.Size(1024,768), serverPort = 0):
    assert isinstance(frame, dk.core.Frame)
    TestApp(frame, size, serverPort).run()

