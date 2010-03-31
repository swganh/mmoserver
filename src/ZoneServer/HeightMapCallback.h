#ifndef HEIGHTMAPCALLBACK_H
#define HEIGHTMAPCALLBACK_H

class HeightmapAsyncContainer;

class HeightMapCallBack
{
public:
	virtual void heightMapCallback(HeightmapAsyncContainer* ref) {}
};

#endif