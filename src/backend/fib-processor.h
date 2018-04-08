/*
 *    Copyright (C) 2018
 *    Matthias P. Braendli (matthias.braendli@mpb.li)
 *
 *    Copyright (C) 2013
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Programming
 *
 *    This file is part of the SDR-J (JSDR).
 *    SDR-J is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    SDR-J is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with SDR-J; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#ifndef FIB_PROCESSOR
#define FIB_PROCESSOR

#include    <vector>
#include    <array>
#include    <mutex>
#include    <cstdint>
#include    <cstdio>
#include    "msc-handler.h"
#include    "radio-controller.h"

struct dabLabel {
    std::string label; // UTF-8 encoded
    uint8_t     mask = 0x00;
};


//  from FIG1/2
struct Service {
    uint32_t serviceId = 0;
    dabLabel serviceLabel;
    bool     hasPNum = false;
    bool     hasLanguage = false;
    int16_t  language = -1;
    int16_t  programType = 0;
    uint16_t pNum = 0;
};

//      The service component describes the actual service
//      It really should be a union
struct ServiceComponent {
    int8_t       TMid;           // the transport mode
    uint32_t     SId;            // belongs to the service
    int16_t      componentNr;    // component

    int16_t      ASCTy;          // used for audio
    int16_t      PS_flag;        // use for both audio and packet
    int16_t      subchannelId;   // used in both audio and packet
    uint16_t     SCId;           // used in packet
    uint8_t      CAflag;         // used in packet (or not at all)
    int16_t      DSCTy;          // used in packet
    uint8_t      DGflag;         // used for TDC
    int16_t      packetAddress;  // used in packet
};

struct Subchannel {
    int32_t  subChId;
    int32_t  startAddr;
    int32_t  length;
    bool     shortForm;

    // when short-form, UEP:
    int16_t  tableIndex;

    // when long-form:
    // Option 0: EEP-A
    // Option 1: EEP-B
    int32_t  protOption;
    int32_t  protLevel;

    int16_t  language;

    // For subchannels carrying packet-mode service components
    int16_t  fecScheme; // 0=no FEC, 1=FEC, 2=Rfu, 3=Rfu

    // Calculate the effective subchannel bitrate
    int32_t bitrate(void);
};

class   CRadioController;

class   FIBProcessor {
    public:
        FIBProcessor(RadioControllerInterface& mr);

        // called from the demodulator
        void    process_FIB(uint8_t*, uint16_t);
        void    clearEnsemble(void);
        bool    syncReached(void);

        // returns PACKET_SERVICE or AUDIO_SERVICE or UNKNOWN_SERVICE
        uint8_t kindofService(const std::string& label);

        // Called from the frontend
        audiodata_t getAudioServiceData(const std::string& label);
        packetdata_t getDataServiceData(const std::string& label);

        std::string getEnsembleName(void) const;
        std::vector<Service> getServiceList(void) const;


    private:
        RadioControllerInterface& myRadioInterface;
        Service *findServiceId(uint32_t serviceId);
        ServiceComponent *find_packetComponent(int16_t SCId);

        void bind_audioService(
                int8_t TMid,
                uint32_t SId,
                int16_t compnr,
                int16_t subChId,
                int16_t ps_flag,
                int16_t ASCTy);

        void bind_packetService(
                int8_t TMid,
                uint32_t SId,
                int16_t compnr,
                int16_t SCId,
                int16_t ps_flag,
                int16_t CAflag);

        void process_FIG0(uint8_t *);
        void process_FIG1(uint8_t *);
        void FIG0Extension0(uint8_t *);
        void FIG0Extension1(uint8_t *);
        void FIG0Extension2(uint8_t *);
        void FIG0Extension3(uint8_t *);
        void FIG0Extension5(uint8_t *);
        void FIG0Extension8(uint8_t *);
        void FIG0Extension9(uint8_t *);
        void FIG0Extension10(uint8_t *);
        void FIG0Extension13(uint8_t *);
        void FIG0Extension14(uint8_t *);
        void FIG0Extension16(uint8_t *);
        void FIG0Extension17(uint8_t *);
        void FIG0Extension18(uint8_t *);
        void FIG0Extension19(uint8_t *);
        void FIG0Extension21(uint8_t *);
        void FIG0Extension22(uint8_t *);

        int16_t HandleFIG0Extension1(uint8_t *d, int16_t offset, uint8_t pd);

        int16_t HandleFIG0Extension2(
                uint8_t *d,
                int16_t offset,
                uint8_t cn,
                uint8_t pd);

        int16_t HandleFIG0Extension3(uint8_t *d, int16_t used);
        int16_t HandleFIG0Extension5(uint8_t *d, int16_t offset);
        int16_t HandleFIG0Extension8(uint8_t *d, int16_t used, uint8_t pdBit);
        int16_t HandleFIG0Extension13(uint8_t *d, int16_t used, uint8_t pdBit);
        int16_t HandleFIG0Extension22(uint8_t *d, int16_t used);

        bool timeOffsetReceived = false;
        dab_date_time_t dateTime = {};
        mutable std::mutex mutex;
        std::string ensembleName;
        std::vector<Subchannel> subChannels;
        std::vector<ServiceComponent> components;
        std::vector<Service> services;
        bool firstTime = true;
        bool isSynced = false;
};

#endif

