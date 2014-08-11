#include "maskedimage.h"

void MaskedImage::setup(ofImage in_texture, ofImage in_mask) {
    //set the texture parameters for the maks shader. just do this at the beginning
	shader.load("masked");
	shader.begin();
    texture = in_texture;
    mask = in_mask;
    
	shader.setUniformTexture("Tex0", texture.getTextureReference(), 0);
	shader.setUniformTexture("Tex1", mask.getTextureReference(), 1);
	shader.end();
}

void MaskedImage::draw(ofImage in_texture, ofImage in_mask) {
    texture = in_texture;
    mask = in_mask;
    //then draw a quad for the top layer using our composite shader to set the alpha
	shader.begin();
    
	//our shader uses two textures, the top layer and the alpha
	//we can load two textures into a shader using the multi texture coordinate extensions
	glActiveTexture(GL_TEXTURE0_ARB);
	texture.getTextureReference().bind();
    
	glActiveTexture(GL_TEXTURE1_ARB);
	mask.getTextureReference().bind();
    
	//draw a quad the size of the frame
	glBegin(GL_QUADS);
    
	//move the mask around with the mouse by modifying the texture coordinates
	glMultiTexCoord2d(GL_TEXTURE0_ARB, 0, 0);
	glMultiTexCoord2d(GL_TEXTURE1_ARB, 0, 0);
	glVertex2f( 0, 0);
    
	glMultiTexCoord2d(GL_TEXTURE0_ARB, texture.getWidth(), 0);
	glMultiTexCoord2d(GL_TEXTURE1_ARB, mask.getWidth(), 0);
	glVertex2f( ofGetWidth(), 0);
    
	glMultiTexCoord2d(GL_TEXTURE0_ARB, texture.getWidth(), texture.getHeight());
	glMultiTexCoord2d(GL_TEXTURE1_ARB, mask.getWidth(), mask.getHeight());
	glVertex2f( ofGetWidth(), ofGetHeight());
    
	glMultiTexCoord2d(GL_TEXTURE0_ARB, 0, texture.getHeight());
	glMultiTexCoord2d(GL_TEXTURE1_ARB, 0, mask.getHeight());
	glVertex2f( 0, ofGetHeight() );
    
	glEnd();
    
	//deactive and clean up
	glActiveTexture(GL_TEXTURE1_ARB);
	mask.getTextureReference().unbind();
    
	glActiveTexture(GL_TEXTURE0_ARB);
	texture.getTextureReference().unbind();
    
	shader.end();
}