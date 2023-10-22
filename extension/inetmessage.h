//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: INetMessage interface
//
// $NoKeywords: $
//=============================================================================//

#ifndef INETMESSAGE_FRAEVEN_H
#define INETMESSAGE_FRAEVEN_H

#include "smsdk_ext.h"
#include "bitbuf.h"
#include "inetchannelinfo.h"

class INetMsgHandler;
class INetMessage;
class INetChannel;

// typedef bool (INetMsgHandler::*PROCESSFUNCPTR)(INetMessage*);
// #define CASTPROCPTR( fn ) static_cast <bool (INetMsgHandler::*)(INetMessage*)> (fn)

class INetMessage
{
public:
	virtual	~INetMessage() {};

	// Use these to setup who can hear whose voice.
	// Pass in client indices (which are their ent indices - 1).

	virtual void	SetNetChannel(INetChannel * netchan) = 0; // netchannel this message is from/for
	virtual void	SetReliable( bool state ) = 0;	// set to true if it's a reliable message

	virtual bool	Process( void ) = 0; // calles the recently set handler to process this message

	virtual	bool	ReadFromBuffer( bf_read &buffer ) = 0; // returns true if parsing was OK
	virtual	bool	WriteToBuffer( bf_write &buffer ) = 0;	// returns true if writing was OK

	virtual bool	IsReliable( void ) const = 0;  // true, if message needs reliable handling

	virtual int				GetType( void ) const = 0; // returns module specific header tag eg svc_serverinfo
	virtual int				GetGroup( void ) const = 0;	// returns net message group of this message
	virtual const char		*GetName( void ) const = 0;	// returns network message name, eg "svc_serverinfo"
	virtual INetChannel		*GetNetChannel( void ) const = 0;
	virtual const char		*ToString( void ) const = 0; // returns a human readable string about message content
};

class CNetMessage : public INetMessage
{
public:
	CNetMessage() {	m_bReliable = true;
					m_NetChannel = NULL; }

	virtual ~CNetMessage() {};

	virtual int		GetGroup() const { return INetChannelInfo::GENERIC; }
	INetChannel		*GetNetChannel() const { return m_NetChannel; }

	virtual void	SetReliable( bool state) {m_bReliable = state;};
	virtual bool	IsReliable() const { return m_bReliable; };
	virtual void    SetNetChannel(INetChannel * netchan) { m_NetChannel = netchan; }
	virtual bool	Process() { return false; };	// no handler set

protected:
	bool				m_bReliable;	// true if message should be send reliable
	INetChannel			*m_NetChannel;	// netchannel this message is from/for
};

#define svc_VoiceData		15

#define DECLARE_BASE_MESSAGE( msgtype )						\
	public:													\
		bool			ReadFromBuffer( bf_read &buffer );	\
		bool			WriteToBuffer( bf_write &buffer );	\
		const char		*ToString() const;					\
		int				GetType() const { return msgtype; } \
		const char		*GetName() const { return #msgtype;}\

#define DECLARE_SVC_MESSAGE( name )		\
	DECLARE_BASE_MESSAGE( svc_##name );	\
	IServerMessageHandler *m_pMessageHandler;\
	bool Process() { return m_pMessageHandler->Process##name( this ); }\

class SVC_VoiceData : public CNetMessage
{
	DECLARE_SVC_MESSAGE( VoiceData );

	int	GetGroup() const { return INetChannelInfo::VOICE; }

	SVC_VoiceData() { m_bReliable = false; }

    public:
        int				m_nFromClient;	// client who has spoken
        bool			m_bProximity;
        int				m_nLength;		// data length in bits
        uint64			m_xuid;			// X360 player ID

        bf_read			m_DataIn;
        void			*m_DataOut;
};

#define NETMSG_TYPE_BITS	6

bool SVC_VoiceData::WriteToBuffer( bf_write &buffer )
{
	buffer.WriteUBitLong( GetType(), NETMSG_TYPE_BITS );
	buffer.WriteByte( m_nFromClient );
	buffer.WriteByte( m_bProximity );
	buffer.WriteWord( m_nLength );

	return buffer.WriteBits( m_DataOut, m_nLength );
}

bool SVC_VoiceData::ReadFromBuffer( bf_read &buffer )
{
	// VPROF( "SVC_VoiceData::ReadFromBuffer" );

	m_nFromClient = buffer.ReadByte();
	m_bProximity = !!buffer.ReadByte();
	m_nLength = buffer.ReadWord();

	// if ( IsX360() )
	// {
		// m_xuid =  buffer.ReadLongLong();
	// }

	m_DataIn = buffer;
	return buffer.SeekRelative( m_nLength );
}

const char *SVC_VoiceData::ToString(void) const
{
	// Q_snprintf(s_text, sizeof(s_text), "%s: client %i, bytes %i", GetName(), m_nFromClient, Bits2Bytes(m_nLength) );
	// return s_text;
    return "idc";
}

#endif

