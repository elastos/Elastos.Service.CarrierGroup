package com.qcode.chatrobot.common;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;

import com.qcode.chatrobot.ui.QRCode;

public class MyQRCode extends QRCode {
    public MyQRCode(int typeNumber, int errorCorrectLevel) {
        super(typeNumber, errorCorrectLevel);
    }
    public Bitmap getBitmap(String src, int width, int height) {
        addData(src);
        make();
        //1.创建Bitmap
        Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas();
        canvas.setBitmap(bitmap);
        // compute tileW/tileH based on options.width/options.height
        float tileW = (float) width / (getModuleCount() + 4);
        float tileH = (float) height / (getModuleCount() + 4);
        Paint paint = new Paint();
        paint.setStyle(Paint.Style.FILL);
        
        // draw in the canvas
        for (int row = 0; row < getModuleCount() + 4; row++) {
            for (int col = 0; col < getModuleCount() + 4; col++) {
                int color = 0xffffffff;
                
                if (row < 2 || row > (getModuleCount() + 1) || col < 2 || col > (getModuleCount() + 1)) {
                
                } else {
                    color = isDark(row - 2, col - 2) ? 0xff000000 : 0xffffffff;
                }
                paint.setColor(color);
                float w = (float) ((Math.ceil((col + 1) * tileW) - Math.floor(col * tileW)));
                float h = (float) ((Math.ceil((row + 1) * tileW) - Math.floor(row * tileW)));
                float x = (float) Math.round(col * tileW);
                float y = (float) Math.round(row * tileH);
                canvas.drawRect(x, y, x + w, y + h, paint);
            }
        }
        return bitmap;
    }
}