class VDP():
    def __init__(self):
        self.status = 0

    def init(self):
        '''
        app = QApplication(sys.argv)
        image = QImage(320, 224, QImage.Format_RGB32)

        value = qRgb(189, 149, 39)  # 0xffbd9527
        image.setPixel(1, 1, value)

        value = qRgb(122, 163, 39)  # 0xff7aa327
        image.setPixel(0, 1, value)
        image.setPixel(1, 0, value)

        value = qRgb(237, 187, 51)  # 0xffedba31
        image.setPixel(2, 1, value)

        #label = QLabel("Hello World")
        #label.setPixmap(QPixmap.fromImage(image))
        #label.show()
        clock = DigitalClock()
        clock.show()
        app.exec_()
        '''
        print 'init vdp'

    def lines(self):
        return xrange(224)

    def invisible_lines(self):
        return xrange(262-224)

    def set_hblank(self):
        self.status |= 4

    def clear_hblank(self):
        self.status &= 0xfffb

    def set_vblank(self):
        self.status |= 8

    def clear_vblank(self):
        self.status &= 0xfff7
        self.status |= 0x80
