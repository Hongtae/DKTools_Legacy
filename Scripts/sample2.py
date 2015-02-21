import dk
import math
    
sprite = dk.sprite

class Frame(dk.ui.View):
    def onLoaded(self):
        super().onLoaded()

        resDir = dk.appInstance().resourceDir
        self.resourcePool = dk.ResourcePool()
        self.resourcePool.addSearchPath(resDir)
        self.resourcePool.addSearchPath(resDir + '/Fonts')
        self.resourcePool.addSearchPath(resDir + '/Sprite')

        # 폰트 로딩
        fontData = self.resourcePool.loadResourceData('NanumGothic.ttf')
        self.textFont = dk.Font(fontData, 30, dpi = (72,72))
        self.outlineFont = dk.Font(fontData, 30, dpi = (72,72), outline = 1)
        self.defaultFont = dk.Font(fontData, 24)
        self.mousePositions = {}

        # texture-pack-atlas
        tpack = self.resourcePool.openResourceStream('MainUI.plist')
        self.texAtlas = dk.sprite.TexturePack.createFromPlist(tpack, self.resourcePool)
        #print('texAtlas[name]: ', self.texAtlas.filename)
        #print('texAtlas[size]: ', self.texAtlas.resolution)
        #print('texAtlas[frames]: ', self.texAtlas.frames)

        bounds = self.bounds()
        self.sprite = sprite.Sprite(bounds.center, bounds.size, name='Scene Root')
        sprite1 = sprite.Sprite(self.sprite.bounds().center, (640,385), self.texAtlas, 'characterSet02.png')
        sprite2 = sprite.Sprite((300,300), (161,679), self.texAtlas, 'light.png')
        
        anim = sprite.Animation(0, 20).addFrame(5, math.pi*0.5).addFrame(10, math.pi).addFrame(20, math.pi*2)
        anim.repeat = 0x7fffffff
        sprite2.setAnimation('rotate', anim)

        sprite3 = sprite.Sprite((80,400), (346,127), self.texAtlas, 'play_btn.png')
        sprite3.buttonCallback = self.onPlayButton
        sprite3.setTextureIds(sprite.Sprite.STATE_HIGHLIGHTED, 'play_btn_down.png')
        sprite2.addChild(sprite3)

        sprite1.addChild(sprite2)

        self.sprite.addChild(sprite1)
        

    def onPlayButton(self, sender):
        print('onPlayButton, type(sender): ', type(sender))

    def onResized(self):
        super().onResized()
        print('MainFrame.onResized')
        bounds = self.bounds()
        self.sprite.center = bounds.center
        self.sprite.size = bounds.size
        sprite1 = self.sprite.children[0]
        sprite1.center = self.sprite.bounds().center

    def onUnload(self):
        super().onUnload()
        print('MainFrame.onUnload')
        del self.textFont
        del self.outlineFont
        del self.defaultFont
        del self.mousePositions
        del self.resourcePool
        del self.texAtlas
        del self.sprite

    def onUpdate(self, delta, tick, date):
        super().onUpdate(delta, tick, date)
        self.redraw()
        self.delta = delta
        self.tick = tick
        self.date = date

        self.sprite.update(delta, tick)

    def onRender(self, renderer):
        renderer.clear(dk.color.blue)

        baseline = self.convertPixelToLocal(dk.Point(0, self.textFont.lineHeight() - self.textFont.baseline))
        baseline.y = 1.0 - baseline.y

        with renderer.contextForTextBaseline(self.textFont, baseline, renderer.TEXT_ALIGN_LEFT_DOWNWARD) as text, \
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

        self.sprite.render(renderer)

    def onMouseDown(self, deviceId, buttonId, pos):
        self.sprite.mouseDown(deviceId, self.sprite.convertPoint(pos))

        if buttonId is 0:
            self.mousePositions[deviceId] = pos
        return super().onMouseDown(deviceId, buttonId, pos)

    def onMouseUp(self, deviceId, buttonId, pos):
        self.sprite.mouseUp(deviceId, self.sprite.convertPoint(pos))

        if buttonId is 0 and deviceId in self.mousePositions:
            del(self.mousePositions[deviceId])
        return super().onMouseUp(deviceId, buttonId, pos)

    def onMouseMove(self, deviceId, pos, delta):
        self.sprite.mouseMove(deviceId, self.sprite.convertPoint(pos))

        if deviceId in self.mousePositions:
            self.mousePositions[deviceId] = pos
        return super().onMouseMove(deviceId, pos, delta)
