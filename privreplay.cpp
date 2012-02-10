#include "User.h"
#include <sys/stat.h>

class CPrivReplay : public CModule
{
public:
    MODCONSTRUCTOR(CPrivReplay)
    {

    }

    virtual ~CPrivReplay()
    {

    }

    virtual EModRet OnPrivMsg(CNick& Nick, CString& sMessage)
    {
        CString msg = m_pUser->AddTimestamp(CString("[->] ") + sMessage);
        StoreMessage(Nick, msg);

        return (CONTINUE);
    }

    virtual EModRet OnUserRaw(CString &sLine)
    {
        //PutModule("Raw: " + sLine);
        if (sLine.Left(7).Equals("PRIVMSG"))
        {
            //PutModule("OUTBOUND PRIVMSG");
            VCString vsRet;
            sLine.Split(" ", vsRet);

            // Check the prefix on the to name and ignore special users, control channels and channels.
            CString toNamePrefix = vsRet[1].Left(1);
            if (toNamePrefix.Equals("*") || toNamePrefix.Equals("&") || toNamePrefix.Equals("#"))
            {
                return CONTINUE;
            }

            CString msg = vsRet[2];
            msg.LeftChomp(1);
            
            // Don't process outgoing CTCP
            if (msg.Left(1).Equals("\x01"))
              return (CONTINUE);
            
            for (int c = 3; vsRet.size() > c; c++)
            {
                msg += " " + vsRet[c];
            }

            StoreMessage(":" + vsRet[1] + " PRIVMSG " + m_pUser->GetIRCNick().GetNick() + " :[<-] " + msg);
        }
        return (CONTINUE);
    }

    virtual void OnClientLogin()
    {
        ReplayMessages();
    }

private:
    void StoreMessage(const CNick & Nick, CString & sMessage)
    {
        //PutModule("Store: " + Nick.GetNick() + " -> " + sMessage);
        if (m_pUser && Nick.GetNick() == m_pUser->GetIRCNick().GetNick())
        {
            // StoreMessage(":" + m_pUser->GetIRCNick().GetNick() + " PRIVMSG " + Nick.GetNickMask() + " :" + sMessage);
            //StoreMessage(":" + Nick.GetNickMask() + " PRIVMSG " + m_pUser->GetIRCNick().GetNick() + " :" + sMessage);
        }
        else
        {
            StoreMessage(":" + Nick.GetNickMask() + " PRIVMSG " + m_pUser->GetIRCNick().GetNick() + " :" + sMessage);
        }
    }

    void StoreMessage(const CString & sText)
    {
        m_vMessages.push_back(sText);
    }

    void ReplayMessages()
    {
        if (!m_vMessages.empty())
        {
            vector<CString>::iterator iter;
            for (iter = m_vMessages.begin(); iter != m_vMessages.end(); iter++)
            {
                PutUser(*iter);
            }
        }
    }

    vector<CString> m_vMessages;
};

MODULEDEFS(CPrivReplay, "Stores private messages and replays them");

