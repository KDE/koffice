# This file is part of Krita
#
# Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

require "krosskritacore"

class TorturePainting

    def initialize()
    
        doc = Krosskritacore::get("KritaDocument")
        @script = Krosskritacore::get("KritaScript")
        
        @image = doc.getImage()
        @width = @image.getWidth()
        @height = @image.getHeight()
        
        @script.setProgressTotalSteps(30 * 10)
        
        testColorspace("RGBA")
        testColorspace("RGBA16")
        testColorspace("RGBAF16HALF")
        testColorspace("RGBAF32")
        testColorspace("CMYK")
        testColorspace("CMYKA16")
        testColorspace("CMYK")
        testColorspace("CMYKA16")
        testColorspace("LABA")
        testColorspace("LMSAF32")
        
    
    end
    
    def randomizeStyle(painter)
        painter.setFillStyle(4 *rand)
        painter.setStrokeStyle(2 *rand)
    end
    
    
    def testColorspace(cs)
        print "Torturing for ", cs, "\n"
        layer = @image.createPaintLayer("torture", 255 * rand, "RGBA" );
        torture(layer)
    end
    
    
    def torture(layer)
        layer.beginPainting("torture painting")
        
        painter = layer.createPainter()
        
        # create painting color
        blackcolor = Krosskritacore::newRGBColor(0,0,0)
        
        # set painting color
        painter.setPaintColor( blackcolor )
        
        # get the pattern
        pattern = Krosskritacore::getPattern("Bricks")
        
        # set the pattern
        painter.setPattern(pattern)
        
        # define the paint operation
        painter.setPaintOp("paintbrush")
        
        # randomly rect or circle paint
        for i in 1..30
            # set painting color
            painter.setPaintColor( Krosskritacore::newRGBColor(rand*255,rand*255,rand*255) )
            painter.setBackgroundColor( Krosskritacore::newRGBColor(rand*255,rand*255,rand*255) )
            painter.setOpacity( rand*255 )
            # set the brush
            if(rand < 0.5)
                painter.setBrush( Krosskritacore::newRectBrush(rand*20,rand*20,rand*10,rand*10) )
            else
                painter.setBrush( Krosskritacore::newCircleBrush(rand*20,rand*20,rand*10,rand*10) )
            end
            # paint a point
            shape = rand * 7
            painter.setStrokeStyle(1)
            if( shape < 1 )
                painter.paintAt(rand * @width , rand * @height,1.1)
            elsif(shape < 2 )
                xs = Array.new
                ys = Array.new
                for i in 0..6
                    xs[i] = rand*@width
                    ys[i] = rand*@height
                end
                painter.paintPolyline(xs,ys)
            elsif(shape < 3)
                painter.paintLine(rand * @width, rand * @height, 1.1, rand * @width, rand * @height,1.1)
            elsif(shape < 4)
                painter.paintBezierCurve(rand * @width, rand * @height, 1.1, rand * @width, rand * @height, rand * @width , rand * @height, rand * @width, rand * @height, 1.1)
            elsif(shape < 5)
                randomizeStyle(painter)
                painter.paintEllipse(rand * @width, rand * @height, rand * @width, rand * @height, 1.1)
            elsif(shape < 6)
                xs = Array.new
                ys = Array.new
                for i in 0..6
                    xs[i] = rand*@width
                    ys[i] = rand*@height
                end
                randomizeStyle(painter)
                painter.paintPolygon(xs, ys)
            elsif(shape < 7)
                randomizeStyle(painter)
                painter.paintRect(rand * @width, rand * @height, rand * @width, rand * @height, 1.1)
            end
            @script.incProgress()
        end
        layer.endPainting()
    end

end

TorturePainting.new()