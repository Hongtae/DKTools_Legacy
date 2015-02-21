import dk
import gc

from sample1 import Frame as Sample1
from sample2 import Frame as Sample2
from sample3 import Frame as Sample3
from sample4 import Frame as Sample4

# dk.ui.view.DEFAULT_UI_SCALE = 2

class Sample(dk.ui.View):
    def __init__(self, frames):
        super().__init__()


    def onCloseSample(self, button):
        if self.demoFrame:
            self.demoFrame.unload()
            self.demoFrame.removeFromParent()
            self.demoFrame = None
            self.screen().postOperation(self.runGC, ())
        else:
            self.screen().window.close()

    def runGC(self):
        print('running GC')
        items = gc.collect()
        print('gc.collect() returns: ', items)

    def onSelectSample(self, button):
        assert self.demoFrame == None

        cls = button.sampleFrameClass
        print('create sample frame: ', cls.__name__)
        self.demoFrame = button.sampleFrameClass(frame=self.bounds())
        if isinstance(self.demoFrame, dk.ui.View):
            self.demoFrame.frame = self.contentBounds()
        else:
            t = dk.LinearTransform2()
            t.scale( self.contentScale )
            self.demoFrame.transform = dk.AffineTransform2(t).matrix3()

        self.addChild(self.demoFrame)
        self.bringChildToFront(self.closeButton)
        self.closeButton.hidden = False

    def onLoaded(self):
        super().onLoaded()

        dk.ui.Label.fontAttributes = dk.ui.font.attributes(18)
        dk.ui.Button.fontAttributes = dk.ui.font.attributes(18)
        dk.ui.resource.clear()

        offset = 100

        self.demoFrame = None

        for name,cls in frames:
            button = dk.ui.Button( name, frame=dk.Rect(50,offset, 400, 50))
            button.addTarget(self, self.onSelectSample)
            button.sampleFrameClass = cls
            self.addChild(button)
            offset += 60

        self.closeButton = dk.ui.Button('Close', frame=dk.Rect(0,0,80,40))
        self.closeButton.setTextColor(dk.Color(1,1,1,1))
        self.closeButton.setOutlineColor(dk.Color(0,0,0,1))
        self.closeButton.drawTextOutline = True
        self.closeButton.backgroundColor = dk.Color(1,0,0,0.75)
        self.closeButton.backgroundColorHighlighted = dk.Color(1,0.5,0.5)
        self.closeButton.backgroundColorActivated = dk.Color(0.8,0,0)
        self.closeButton.addTarget(self, self.onCloseSample)
        self.addChild(self.closeButton)

        self.screen().postOperation(self.runGC, ())

    def onUnload(self):
        self.demoFrame = None
        self.closeButton = None

        for c in self.children():
            c.unload()
            c.removeFromParent()

        super().onUnload()

        gc.collect()

        app = dk.App.instance()
        if app:
            app.terminate(0)
        else:
            raise SystemExit

    def onResized(self):
        super().onResized()
        if self.demoFrame:
            if isinstance(self.demoFrame, dk.ui.View):
                self.demoFrame.frame = self.contentBounds()
            else:
                t = dk.LinearTransform2()
                t.scale( self.contentScale )
                self.demoFrame.transform = dk.AffineTransform2(t).matrix3()

        btnSize = (80, 40)
        padding = 5
        width, height = self.contentScale
        btnRect = dk.Rect(width-btnSize[0]-padding, padding, *btnSize)
        self.closeButton.frame = btnRect


if __name__ == '__main__':
    import common

    frames = [('Simple Texture Drawing', Sample1),
              ('Sprite Test', Sample2),
              ('UI Test', Sample3),
              ('Physics Test', Sample4)
              ]

    common.runApp( Sample( frames ), size=dk.Size(640,480))
