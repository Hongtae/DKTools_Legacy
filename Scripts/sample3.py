import dk
import math

ui = dk.ui
Rect = dk.Rect
Size = dk.Size
Point = dk.Point

class Frame(dk.ui.View):
    def onLoaded(self):
        super().onLoaded()
        print('self.contentScale: ', self.contentScale)

        resDir = dk.appInstance().resourceDir
        resourcePool = dk.ResourcePool()
        resourcePool.addSearchPath(resDir)
        texture = resourcePool.loadResource('koo.jpg')

        ui.Label.fontAttributes = ui.font.attributes(18)
        ui.Button.fontAttributes = ui.font.attributes(18)
        ui.resource.clear()

        self.backgroundColor = dk.Color(0.65, 0.72, 1)

        label = ui.Label('Test Label 1234', Rect(10,100,200,80))
        label.align = ui.label.ALIGN_BOTTOM_LEFT
        label.linebreak = ui.label.LINE_BREAK_TRUNCATING_TAIL
        label.borderWidth = 1

        label2 = ui.Label('Test Label 1234', Rect(10,200,200,80))
        label2.align = ui.label.ALIGN_BOTTOM_LEFT
        label2.linebreak = ui.label.LINE_BREAK_TRUNCATING_TAIL
        label2.borderWidth = 0


        button1 = ui.Button('button 1', Rect(10,300,200,80))
        button2 = ui.Button('button 2', Rect(300,300,200,80))

        button1.backgroundImage = texture
        button1.borderWidth = 1

        self.buttons = (button1, button2)
        for btn in self.buttons:
            btn.align = ui.label.ALIGN_CENTER
            btn.drawTextOutline = True
            btn.setTextColor(dk.color.white)
            btn.setOutlineColor(dk.Color(0,0,0,0.9))
            # btn.backgroundColor = dk.Color(0.75, 0.75, 0.75)
            # btn.activatedColor = dk.Color(0.42, 0.42, 0.42)
            # btn.hoverColor = dk.Color(0.85, 0.85, 1.0)
            btn.addTarget(self, self.onButtonClick)

        class ImageScrollView(ui.ScrollView, ui.ImageView):
            pass

        scrollView = ImageScrollView(image=texture, contentSize=Size(300,300), frame=Rect(300,80,200,180))
        if scrollView:
            scrollView.backgroundColor = dk.Color(0.5, 0.5, 1.0)
            scrollView.borderWidth = 1
            #scrollView.textureColor = dk.Color(1,0,0)
            child = ui.Label('ScrollView Label', frame=Rect(10,10,160,50))
            child.backgroundColor = dk.Color(1,0,0)
            child.enabled = False
            scrollView.addChild(child)


        self.addChild(label)
        self.addChild(label2)
        self.addChild(button1)
        self.addChild(button2)
        self.addChild(scrollView)

        self.infoLabels = (ui.Label(), ui.Label(), ui.Label())
        for info in self.infoLabels:
            info.align = ui.label.ALIGN_BOTTOM_LEFT
            info.backgroundColor = dk.Color(0,0,0,0.2)
            info.textColor = dk.Color(1,1,1,1)
            info.outlineColor = dk.Color(0,0,0,0.85);
            info.drawTextOutline = True
            info.setBlendState(dk.blendstate.defaultAlpha)
            self.addChild(info)

        self.screen().postOperation(self.onResized, ())
        
    def onResized(self):
        super().onResized()
        print('MainFrame.onResized')

        offset = 0
        width = self.contentResolution[0]
        for label in self.infoLabels:
            if label.font:
                height = label.font.lineHeight()
                frame = self.convertPixelToLocal(Rect(0, offset, width, height))
                label.frame = frame
                offset += height

    def onUnload(self):
        super().onUnload()
        print('MainFrame.onUnload')
        self.infoLabels = ()
        self.buttons = ()
        for c in self.children():
            if isinstance(c, ui.Button):
                c.removeTarget(self)

    def onUpdate(self, delta, tick, date):
        super().onUpdate(delta, tick, date)

        appInfo = dk.appInstance().appInfo
        hostName = appInfo[dk.app.APP_INFO_HOST_NAME]
        osName = appInfo[dk.app.APP_INFO_OS_NAME]
        userName = appInfo[dk.app.APP_INFO_USER_NAME]

        text0 = ' ' + osName
        text1 = ' {}@{}'.format(userName, hostName)
        text2 = ' Elapsed: {0:.6f} ({1:.2f} FPS) {2[0]:.0f}x{2[1]:.0f}'.format(delta, 1.0/delta, self.contentScale)

        for label, text in zip(self.infoLabels, (text0, text1, text2)):
            label.text = text
            label.redraw()

    def onRender(self, renderer):
        super().onRender(renderer)

    def onButtonClick(self, btn):
        if isinstance(btn, ui.Button):
            print('onButtonClick: ', btn.text)
        else:
            print('onButtonClick: ', type(btn))

        if btn == self.buttons[0]:
            if self.buttons[1].enabled:
                self.buttons[1].enabled = False
            else:
                self.buttons[1].enabled = True
            self.buttons[1].redraw()
        else:
            uuid = dk.uuidgen()
            print('uuid: ', uuid)

