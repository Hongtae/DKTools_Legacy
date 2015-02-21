import dk
import math

def createRigidBody(mass, shape, transform=dk.NSTransform(), name=''):
    if mass == 0.0:
        localInertia = dk.Vector3(0,0,0)
    else:
        localInertia = shape.calculateLocalInertia(mass)

    rb = dk.RigidBody(shape, mass, localInertia)
    rb.setWorldTransform(transform)
    rb.name = name
    return rb


class CameraInfo:
    def __init__(self, fov = math.radians(80), near=10, far=100, aspect=1.0, target=dk.Vector3(0,0,0)):
        self.fov = fov
        self.near = near
        self.far = far
        self.aspect = aspect
        self.target = target


class SimpleButton(dk.ui.View):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.activated = False

    def onMouseDown(self, deviceId, buttonId, pos):
        if not self.activated:
            self.captured = deviceId
            self.captureMouse(deviceId)
            self.activated = True

    def onMouseUp(self, deviceId, buttonId, pos):
        if hasattr(self, 'captured') and self.captured == deviceId:
            self.releaseMouse(deviceId)
            self.activated = False

class Frame(dk.ui.View):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.backgroundColor = dk.Color(0.6, 0.8, 1.0)
        self.camera = dk.Camera()
        self.cameraInfo = CameraInfo(fov = math.radians(80),
                                     near=10.0,
                                     far=1000.0,
                                     aspect=1.0,
                                     target=dk.Vector3(0,0,0))
        pos = dk.Vector3(0,20,-20)
        up = dk.Vector3(0,1,0)
        self.camera.setView(pos, self.cameraInfo.target - pos, up)
        self.steering = 0

    def resetCamera(self, *args):
        carTrans = self.carChassis.worldTransform()
        pos = dk.Vector3(0, 20, -20)
        pos.transform(carTrans)
        target = dk.Vector3(carTrans.position)
        self.cameraInfo.target = target
        self.camera.setView(pos, target-pos, dk.Vector3(0,1,0))
        self.updatePerspective()

        print('pos3: ', pos)
        print('target3: ', target)


    def updatePerspective(self):
        w,h = self.contentResolution
        self.cameraInfo.aspect = w/h
        self.camera.setPerspective(self.cameraInfo.fov, self.cameraInfo.aspect, self.cameraInfo.near, self.cameraInfo.far)


    def updateView(self):
        pos = self.camera.position()
        #up = self.camera.up()
        up = dk.Vector3(0,1,0)
        dir = self.cameraInfo.target - pos
        self.camera.setView(pos, dir, up)

    def shootBox(self, start, destination, speed=30):

        n = dk.random() % len(self.shootingShapes)

        box = createRigidBody(25.0, self.shootingShapes[n], start, name='a box')
        box.setLinearFactor(dk.Vector3(1,1,1))
        linVel = destination - start
        linVel.normalize()
        linVel *= speed

        box.setLinearVelocity(linVel)
        box.setAngularVelocity(dk.Vector3(0,0,0))

        self.scene.addObject(box)


    def onResized(self):
        super().onResized()
        self.updatePerspective()
        bounds = self.bounds()
        self.brakeButton.frame = dk.Rect(20, 100, 80, 80)
        self.accelButton.frame = dk.Rect(120, 100, 80, 80)
        self.leftButton.frame = dk.Rect(bounds.width - 220, 100, 80, 80)
        self.rightButton.frame = dk.Rect(bounds.width - 100, 100, 80, 80)

        self.steerSlider.frame = dk.Rect(bounds.width - 240, 200, 240, 80)

        self.cameraButton.frame = dk.Rect(10, bounds.height - 50, 140, 40)

        if self.infoLabel.font:
            width = self.contentResolution[0]
            height = self.infoLabel.font.lineHeight()
            frame = self.convertPixelToLocal(dk.Rect(0, 0, width, height+15))
            self.infoLabel.frame = frame

    def resetScene(self):
        self.vehicleSteering = 0.0
        self.carChassis.setLinearVelocity(dk.Vector3(0,0,0))
        self.carChassis.setAngularVelocity(dk.Vector3(0,0,0))


    def onLoaded(self):
        super().onLoaded()

        resourceDir = dk.appInstance().resourceDir
        self.resourcePool = dk.ResourcePool()

        # self.resourcePool.addSearchPath(resourceDir)
        self.resourcePool.addSearchPath(resourceDir)

        # 시뮬레이터 씬 생성
        self.scene = dk.DynamicsScene()
        self.scene.setAmbientColor(dk.color.white)
        self.scene.lights = [dk.light.directional(dk.Vector3(-1,-1,-1), dk.Color(1,1,1))]
        self.scene.updateLights()
        #self.scene.setDrawMode(dk.scene.DRAW_MESHES, dk.scene.DRAW_COLLISION_SHAPES)
        self.scene.drawMode = dk.scene.DRAW_COLLISION_SHAPES
        self.scene.lights.append(dk.Light())

        # 바닥 객체 생성
        groundShape = dk.BoxShape(500,100,500)
        self.groundObject = createRigidBody(0, groundShape, dk.Vector3(0, -100, 0), 'groundObject')

        self.scene.addObject(self.groundObject)

        # 자동차 몸체
        chassisShape = dk.CompoundShape()
        chassisShape.addChild(dk.BoxShape(1, 0.5, 2), dk.Vector3(0,1,0))
        chassisShape.addChild(dk.BoxShape(0.85, 0.3, 0.9), dk.Vector3(0, 1.8, -0.3))

        self.carChassis = createRigidBody(800, chassisShape, name='car chassis')
        #self.carChassis.setWorldTransform(dk.NSTransform(dk.Quaternion(dk.Vector3(0,0,1), math.pi * 0.3), dk.Vector3(0,10,0)))

        self.scene.addObject(self.carChassis)

        self.resetScene()

        # 자동차 객체 생성
        self.vehicle = dk.controller.Vehicle(self.carChassis)
        # 리지드 바디 항상 활성화 시킴
        self.carChassis.keepActivating(True)

        # 씬에 추가
        self.scene.addObject(self.vehicle)

        WHEEL_WIDTH = 0.4
        WHEEL_RADIUS = 0.5

        # 바퀴 쉐이프 생성
        wheelShape = dk.CylinderShape(WHEEL_WIDTH, WHEEL_RADIUS, WHEEL_RADIUS, dk.collisionshape.UP_AXIS_LEFT)
        self.wheelShape = wheelShape
        self.wheelTrans = []

        CONNECTION_HEIGHT = 1.2
        CUBE_HALF_EXTENTS = 1.0

        suspensionResetLength = 0.6
        wheelDirectionCS0 = dk.Vector3(0, -1, 0)
        wheelAxleCS = dk.Vector3(-1, 0, 0)

        tunning = {'suspensionStiffness': 20.0,
                   'suspensionDamping' : 2.3,
                   'suspensionCompression' : 4.4,
                   'frictionSlip' : 1000.0,
                   'rollInfluence' : 0.1}

        xvalues = (CUBE_HALF_EXTENTS-(0.3*WHEEL_WIDTH),
                   -CUBE_HALF_EXTENTS+(0.3*WHEEL_WIDTH),
                   -CUBE_HALF_EXTENTS+(0.3*WHEEL_WIDTH),
                   CUBE_HALF_EXTENTS-(0.3*WHEEL_WIDTH))
        zvalues = (2*CUBE_HALF_EXTENTS-WHEEL_RADIUS,
                   2*CUBE_HALF_EXTENTS-WHEEL_RADIUS,
                   -2*CUBE_HALF_EXTENTS+WHEEL_RADIUS,
                   -2*CUBE_HALF_EXTENTS+WHEEL_RADIUS)

        for x, z in zip(xvalues, zvalues):
            connectionPointCS0 = dk.Vector3(x, CONNECTION_HEIGHT, z)
            print('connectionPointCS0: ', connectionPointCS0)
            self.vehicle.addWheel(connectionPointCS0,
                                  wheelDirectionCS0,
                                  wheelAxleCS,
                                  suspensionResetLength,
                                  WHEEL_RADIUS,
                                  **tunning)

        self.vehicle.resetSuspension()
        for wheel in self.vehicle.wheels:
            self.vehicle.updateWheelTransform(wheel)

        # 슈팅 박스
        self.shootingShapes = (dk.BoxShape(0.5,0.5,0.5),
                               dk.BoxShape(1.5,0.1,1.5),
                               dk.CylinderShape(0.5,1,0.5),
                               dk.ConeShape(2,1),
                               dk.ConeShape(1,2))

        # 버튼 추가
        self.accelButton = SimpleButton(frame=dk.Rect(100, 100, 80, 80))
        self.accelButton.backgroundColor = dk.color.blue
        self.brakeButton = SimpleButton(frame=dk.Rect(10, 100, 80, 80))
        self.brakeButton.backgroundColor = dk.color.red
        self.leftButton = SimpleButton(frame=dk.Rect(420, 100, 80, 80))
        self.leftButton.backgroundColor = dk.color.white
        self.rightButton = SimpleButton(frame=dk.Rect(540, 100, 80, 80))
        self.rightButton.backgroundColor = dk.color.white
        self.cameraButton = dk.ui.Button(text='Reset Camera', frame=dk.Rect(10,10,80,50))
        self.cameraButton.addTarget(self, self.resetCamera)

        self.addChild(self.accelButton)
        self.addChild(self.brakeButton)
        self.addChild(self.leftButton)
        self.addChild(self.rightButton)
        self.addChild(self.cameraButton)

        self.steerSlider = dk.ui.Slider(frame=dk.Rect(200,100,200,50))
        self.addChild(self.steerSlider)
        self.steerSlider.minimumValue = -0.3
        self.steerSlider.maximumValue = 0.3
        self.steerSlider.value = 0.0
        self.steerSlider.addTarget(self, self.onSteerChanged)

        # 정보 표시
        self.infoLabel = dk.ui.Label()
        self.infoLabel.align = dk.ui.label.ALIGN_LEFT
        self.infoLabel.backgroundColor = dk.Color(0,0,0,0.2)
        self.infoLabel.textColor = dk.Color(1,1,1,1)
        self.infoLabel.outlineColor = dk.Color(0,0,0,0.85)
        self.infoLabel.drawTextOutline = True
        self.addChild(self.infoLabel)

        self.captureKeyboard(0)
        #self.resetCamera()
        self.screen().postOperation(self.onResized, ())

    def onUnload(self):
        super().onUnload()
        self.resourcePool = None
        self.vehicle = None
        self.carChassis = None
        self.scene = None
        self.model = None
        self.wheelShape = None
        self.wheelTrans = None
        self.infoLabel = None
        self.shootingShapes = None

    def onSteerChanged(self, slider):
        self.steering = slider.value * -1.0

    def onUpdate(self, delta, tick, date):
        super().onUpdate(delta, tick, date)

        engineForce = 0.0
        brakeForce = 0.0

        window = self.screen().window

        if self.brakeButton.activated or window.isKeyDown(0, dk.vkey.DOWN):
            brakeForce = 100.0
        elif self.accelButton.activated or window.isKeyDown(0, dk.vkey.UP):
            engineForce = 3000.0

        if not self.steerSlider.isActivated():
            steerOffset = 0.0
            if self.leftButton.activated or window.isKeyDown(0, dk.vkey.LEFT):
                steerOffset -= delta * 0.7
            elif self.rightButton.activated or window.isKeyDown(0, dk.vkey.RIGHT):
                steerOffset += delta * 0.7

            steerValue = self.steerSlider.value
            if steerOffset != 0.0:
                steerValue += steerOffset
            else:
                if steerValue > 0.0:
                    steerValue -= delta * 0.3
                    if steerValue < 0.0: steerValue = 0.0
                elif steerValue < 0.0:
                    steerValue += delta * 0.3
                    if steerValue > 0.0: steerValue = 0.0
            self.steerSlider.setValue(steerValue)


        self.vehicle.setSteeringValue(self.steering, 0)
        self.vehicle.setSteeringValue(self.steering, 1)

        self.vehicle.applyEngineForce(engineForce, 2);
        self.vehicle.applyEngineForce(engineForce, 3);

        self.vehicle.setBrake(brakeForce, 2)
        self.vehicle.setBrake(brakeForce, 3)

        self.scene.update(delta, tick)

        numWheels = len(self.vehicle.wheels)
        self.wheelTrans = [None] * numWheels
        for i in range(numWheels):
            wheel = self.vehicle.wheels[i]
            self.vehicle.updateWheelTransform(wheel)
            self.wheelTrans[i] = wheel.worldTransform

        self.cameraInfo.target = dk.Vector3(self.carChassis.worldTransform().position)
        self.updateView()

        self.infoLabel.text = ' Elapsed: {0:.6f} ({1:.2f} FPS) {2[0]:.0f}x{2[1]:.0f}'.format(delta, 1.0/delta, self.contentResolution)
        self.infoLabel.redraw()

        self.redraw()


    def onRender(self, renderer):
        super().onRender(renderer)
        renderer.polygonOffset = 1, 1
        def chassisColor(co):
            if co is self.carChassis:
                return dk.Color(1.0, 1.0, 1.0), dk.Color(0.0, 0.0, 0.0)

        renderer.renderScene(self.scene, self.camera, 0, objectColorCallback=chassisColor)
        with renderer.contextForCollisionShape(self.camera) as r:
            for trans in self.wheelTrans:
                r.add(self.wheelShape, trans, dk.Color(1.0, 1.0, 1.0), dk.Color(0.0, 0.0, 0.0))

    #keyboard, mouse event
    def onMouseDown(self, deviceId, buttonId, pos):
        super().onMouseDown(deviceId, buttonId, pos)
        if deviceId == 0:
            self.captureMouse(deviceId)

    def onMouseUp(self, deviceId, buttonId, pos):
        super().onMouseUp(deviceId, buttonId, pos)
        if deviceId == 0:
            if self.isMouseCapturedBySelf(deviceId):
                self.releaseMouse(deviceId)
        if deviceId == 1 or buttonId == 1:
            x = (pos.x / self.contentScale[0]) * 2.0 - 1.0
            y = (pos.y / self.contentScale[1]) * 2.0 - 1.0

            viewProjInv = self.camera.viewMatrix() * self.camera.projectionMatrix()
            viewProjInv.inverse()

            rayBegin = dk.Vector3(x, y, -1.0)
            rayEnd = dk.Vector3(x, y, 1.0)

            rayBegin.transform(viewProjInv)
            rayEnd.transform(viewProjInv)

            #target = self.cameraInfo.target
            self.shootBox(rayBegin, rayEnd)
            print('shooting Box!!')

    def onMouseMove(self, deviceId, pos, delta):
        super().onMouseMove(deviceId, pos, delta)
        if deviceId == 0:
            if self.isMouseCapturedBySelf(deviceId):
                dir = self.camera.direction()
                #up = self.camera.up()
                up = dk.Vector3(0,1,0)
                left = dir.cross(up)

                dX = delta.x * 0.01
                dY = delta.y * 0.01

                qY = dk.Quaternion(up, -dX)
                qX = dk.Quaternion(left, dY)
                rot = qX * qY

                pos = self.camera.position() - self.cameraInfo.target
                pos.transform(rot)
                pos += self.cameraInfo.target
                minY = self.cameraInfo.target.y + self.cameraInfo.near + 2
                if pos.y < minY: pos.y = minY
                self.camera.setView(pos, self.cameraInfo.target - pos, up)


    def onMouseWheel(self, deviceId, pos, delta):
        super().onMouseWheel(deviceId, pos, delta)
        if deviceId == 0:
            pos = self.camera.position()
            dir = self.cameraInfo.target - pos

            d1 = dir.length()
            d2 = d1 + delta.y

            d2 = max(d2, 25)
            if d1 != d2:
                up = self.camera.up()
                p = pos - self.cameraInfo.target
                p.normalize()
                p *= d2
                p += self.cameraInfo.target

                self.camera.setView(p, self.cameraInfo.target - p, up)


    def onMouseHover(self, deviceId):
        super().onMouseHover(deviceId)

    def onMouseLeave(self, deviceId):
        super().onMouseLeave(deviceId)

    def onKeyDown(self, deviceId, vkey):
        super().onKeyDown(deviceId, vkey)
        if vkey == dk.vkey.SPACE:
            print('speed:{:.3f} Km/h'.format(self.vehicle.speedKmHour))



    def onKeyUp(self, deviceId, vkey):
        super().onKeyUp(deviceId, vkey)

    def onTextInput(self, deviceId, text):
        super().onTextInput(deviceId, text)

    def onTextInputCandidate(self, deviceId, text):
        super().onTextInputCandidate(deviceId, text)
