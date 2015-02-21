import dk
import random
    
class Frame(dk.Frame):
    def onLoaded(self):
        super().onLoaded()
        print('MainFrame.onLoaded')
        self.trans = dk.AffineTransform2()
        self.color = dk.Color(random.random(),
                                 random.random(),
                                 random.random())

        resDir = dk.appInstance().resourceDir
        self.resourcePool = dk.ResourcePool()
        self.resourcePool.addSearchPath(resDir)
        self.resourcePool.addSearchPath(resDir + '/Fonts')

        fontData = self.resourcePool.loadResourceData('NanumGothic.ttf')
        self.textFont = dk.Font(fontData, 30, dpi = (72,72))
        self.outlineFont = dk.Font(fontData, 30, dpi = (72,72), outline = 1)
        self.defaultFont = dk.Font(fontData, 24)
        self.mousePositions = {}
        self.texture1 = self.resourcePool.loadResource('Aurora.jpg')
        self.texture2 = self.resourcePool.loadResource('koo.jpg')
  
    def onResized(self):
        print('MainFrame.onResized')

    def onUnload(self):
        print('MainFrame.onUnload')
        del(self.textFont)
        del(self.outlineFont)
        del(self.defaultFont)
        del(self.texture1)
        del(self.texture2)
        del(self.mousePositions)
        del(self.resourcePool)
        return super().onUnload()

    def onUpdate(self, delta, tick, date):
        self.redraw()
        self.delta = delta
        self.tick = tick
        self.date = date
        trans = dk.LinearTransform2(delta);
        self.trans *= trans

    def onRender(self, renderer):
        renderer.clear(self.color)

        with renderer.contextForTexturedRects(self.texture2, None, dk.Color(1,1,1,0.25)) as ctxt:
            trans = dk.AffineTransform2(self.trans)
            trans.translate(0.5, 0.5)
            ctxt.add(dk.Rect(-0.25,-0.25,0.5,0.5), trans.matrix3())

        baseline = self.convertPixelToLocal(dk.Point(0, self.textFont.lineHeight() - self.textFont.baseline))
        baseline.y = 1.0 - baseline.y

        with renderer.contextForTextBaseline(self.textFont, baseline, renderer.TEXT_ALIGN_LEFT_DOWNWARD) as text,\
                renderer.contextForTextBaseline(self.outlineFont, baseline, renderer.TEXT_ALIGN_LEFT_DOWNWARD) as outline:
            def draw(str):
                outline.add(str, dk.Color(0,0,0,0.85))
                text.add(str, dk.Color(1,1,1,1))
            def endl():
                outline.endline()
                text.endline()

            draw('QWERTY')
            endl()
            draw('가나다라')

        baseline = self.convertPixelToLocal(dk.Point(0, self.defaultFont.baseline + 4))

        with renderer.contextForTextBaseline(self.defaultFont, baseline, renderer.TEXT_ALIGN_LEFT_UPWARD) as ctxt:
            elapsed = '{:.6f}'.format(self.delta)
            fps = '{:.2f}'.format(1.0 / self.delta)
            ctxt.add('Elapsed: ', dk.Color(1,1,1))
            ctxt.add(elapsed, dk.Color(1,1,0))
            ctxt.add(' (', dk.Color(1,1,1))
            ctxt.add(fps, dk.Color(1,1,0))
            ctxt.add('FPS)', dk.Color(1,1,1))
  
            for devId, pos in self.mousePositions.items():
                ctxt.endline()
                ctxt.add("Mouse[{:d}]: ({:.3f}, {:.3f})".format(devId, pos.x, pos.y),
                         dk.Color(1,1,1))

    def onMouseDown(self, deviceId, buttonId, pos):
        if buttonId is 0:
            self.mousePositions[deviceId] = pos
        return super().onMouseDown(deviceId, buttonId, pos)

    def onMouseUp(self, deviceId, buttonId, pos):
        if buttonId is 0 and deviceId in self.mousePositions:
            del(self.mousePositions[deviceId])
        return super().onMouseUp(deviceId, buttonId, pos)

    def onMouseMove(self, deviceId, pos, delta):
        if deviceId in self.mousePositions:
            self.mousePositions[deviceId] = pos
        return super().onMouseMove(deviceId, pos, delta)

    def onMouseEvent(self, event):
        return super().onMouseEvent(event)
 
