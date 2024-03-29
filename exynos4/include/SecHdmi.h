/*
 * Copyright@ Samsung Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
**
** @author Sangwoo, Park(sw5771.park@samsung.com)
** @date   2010-09-10
**
*/

#ifndef __SEC_HDMI_H__
#define __SEC_HDMI_H__

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>

#ifdef BOARD_USE_V4L2
#include "s5p_tvout_v4l2.h"
#else
#include "s5p_tvout.h"
#endif
#if defined(BOARD_USES_FIMGAPI)
#include "sec_g2d.h"
#endif
#include "s3c_lcd.h"
#include "SecBuffer.h"
#include "SecFimc.h"

#include "../libhdmi/libsForhdmi/libedid/libedid.h"
#include "../libhdmi/libsForhdmi/libcec/libcec.h"

#include "../libhdmi/SecHdmi/SecHdmiCommon.h"
#include "../libhdmi/SecHdmi/SecHdmiV4L2Utils.h"

#if defined(BOARD_USES_FIMGAPI)
#include "FimgApi.h"
#endif

#include <linux/fb.h>

#include <hardware/hardware.h>

#include <utils/threads.h>

/* handy sizes */
#define SZ_1K                           0x00000400
#define SZ_4K                           0x00001000
#define SZ_8K                           0x00002000
#define SZ_16K                          0x00004000
#define SZ_64K                          0x00010000
#define SZ_128K                         0x00020000
#define SZ_256K                         0x00040000
#define SZ_512K                         0x00080000

#define SZ_1M                           0x00100000
#define SZ_2M                           0x00200000
#define SZ_4M                           0x00400000
#define SZ_8M                           0x00800000
#define SZ_16M                          0x01000000
#define SZ_32M                          0x02000000
#define SZ_64M                          0x04000000
#define SZ_128M                         0x08000000
#define SZ_256M                         0x10000000
#define SZ_512M                         0x20000000

#define SZ_1G                           0x40000000
#define SZ_2G                           0x80000000

namespace android {

class SecHdmi: virtual public RefBase
{
public :
    enum HDMI_LAYER {
        HDMI_LAYER_BASE   = 0,
        HDMI_LAYER_VIDEO,
        HDMI_LAYER_GRAPHIC_0,
        HDMI_LAYER_GRAPHIC_1,
        HDMI_LAYER_MAX,
    };

private :
    class CECThread: public Thread
    {
        public:
            bool                mFlagRunning;

        private:
            sp<SecHdmi>         mSecHdmi;
            Mutex               mThreadLoopLock;
            Mutex               mThreadControlLock;
            virtual bool        threadLoop();
            enum CECDeviceType  mDevtype;
            int                 mLaddr;
            int                 mPaddr;

        public:
            CECThread(sp<SecHdmi> secHdmi)
                :Thread(false),
                mFlagRunning(false),
                mSecHdmi(secHdmi),
                mDevtype(CEC_DEVICE_PLAYER),
                mLaddr(0),
                mPaddr(0){
            };
            virtual ~CECThread();

            bool start();
            bool stop();

    };

    Mutex        mLock;

    sp<CECThread>               mCECThread;

    bool         mFlagCreate;
    bool         mFlagConnected;
    bool         mFlagLayerEnable[HDMI_LAYER_MAX];
    bool         mFlagHdmiStart[HDMI_LAYER_MAX];

    int          mSrcWidth[HDMI_LAYER_MAX];
    int          mSrcHeight[HDMI_LAYER_MAX];
    int          mSrcColorFormat[HDMI_LAYER_MAX];
    int          mHdmiResolutionWidth[HDMI_LAYER_MAX];
    int          mHdmiResolutionHeight[HDMI_LAYER_MAX];

    int          mHdmiDstWidth;
    int          mHdmiDstHeight;

    unsigned int mHdmiSrcYAddr;
    unsigned int mHdmiSrcCbCrAddr;

    int          mHdmiOutputMode;
    unsigned int mHdmiResolutionValue;

    unsigned int mHdmiPresetId;
    v4l2_std_id  mHdmiStdId;
    unsigned int mCompositeStd;

    bool         mHdcpMode;
    int          mAudioMode;
    unsigned int mUIRotVal;
    unsigned int mG2DUIRotVal;

    unsigned int mUIFlipVal; //yqf test

    int          mCurrentHdmiOutputMode;
    unsigned int mCurrentHdmiResolutionValue;
    bool         mCurrentHdcpMode;
    int          mCurrentAudioMode;
    bool         mHdmiInfoChange;
    int          mFimcDstColorFormat;

    SecBuffer    mFimcReservedMem[HDMI_FIMC_OUTPUT_BUF_NUM];
    unsigned int mFimcCurrentOutBufIndex;
    SecFimc      mSecFimc;

    unsigned int mHdmiResolutionValueList[14];
    int          mHdmiSizeOfResolutionValueList;

    SecBuffer    mMixerBuffer[HDMI_LAYER_MAX][MAX_BUFFERS_MIXER];

    void         *mFBaddr;
    unsigned int mFBsize;
    int          mFBionfd;
    unsigned int mFBIndex;
    int          mHdmiFd[HDMI_LAYER_MAX];

    int          mDstWidth[HDMI_LAYER_MAX];
    int          mDstHeight[HDMI_LAYER_MAX];
    int          mPrevDstWidth[HDMI_LAYER_MAX];
    int          mPrevDstHeight[HDMI_LAYER_MAX];

    int          mDefaultFBFd;
    int          mDisplayWidth;
    int          mDisplayHeight;

    struct v4l2_rect mDstRect;
public:	//added yqf,
    bool         mRotateInfoChange;	
public :

    SecHdmi();
    virtual ~SecHdmi();
    bool        create(int width, int height);
    bool        destroy(void);
    inline bool flagCreate(void) { return mFlagCreate; }

    bool        connect(void);
    bool        disconnect(void);

    bool        flagConnected(void);

    bool        flush(int srcW, int srcH, int srcColorFormat,
                        unsigned int srcYAddr, unsigned int srcCbAddr, unsigned int srcCrAddr,
                        int dstX, int dstY,
                        int hdmiLayer,
                        int num_of_hwc_layer);

	bool        clear(int hdmiLayer);

    bool        setHdmiOutputMode(int hdmiOutputMode, bool forceRun = false);
    bool        setHdmiResolution(unsigned int hdmiResolutionValue, bool forceRun = true);
    bool        setHdcpMode(bool hdcpMode, bool forceRun = false);
    bool        setUIRotation(unsigned int rotVal, unsigned int hwcLayer);
    bool 	    setUIFlip(unsigned int flipVal, unsigned int hwcLayer);	 //yqf test
    bool        setDisplaySize(int width, int height);

private:

    bool        m_reset(int w, int h, int colorFormat, int hdmiLayer, int hwcLayer);
    bool        m_startHdmi(int hdmiLayer, unsigned int num_of_plane);
    bool        m_startHdmi(int hdmiLayer);
    bool        m_stopHdmi(int hdmiLayer);
    bool        m_setHdmiOutputMode(int hdmiOutputMode);
    bool        m_setHdmiResolution(unsigned int hdmiResolutionValue);
    bool        m_setCompositeResolution(unsigned int compositeStdId);
    bool        m_setHdcpMode(bool hdcpMode);
    bool        m_setAudioMode(int audioMode);

    int         m_resolutionValueIndex(unsigned int ResolutionValue);
    bool        m_flagHWConnected(void);
};

}; // namespace android

#endif //__SEC_HDMI_H__
