#if defined __prioritycentertext_0_included
 #endinput
#endif
#define __prioritycentertext_0_included


/*
    PriorityCenterText (Version 0x06)

    Only one message can be shown in center text at a time.
    These stocks allow that space to be given different priority levels that prevent new messages from overwriting what's there.

    By: Chdata

*/

#define MAX_CENTER_TEXT         192            //  PrintCenterText()
#define MAX_INT                 2147483647     //  
#define MIN_INT                 -2147483648    // 
//#define TF_MAX_PLAYERS          34             //   

static s_iLastPriority[MAXPLAYERS+1] = {MIN_INT,...};
static Float:s_flResetPriority[MAXPLAYERS+1] = {0.0,...};

/*
    An example of how to use this:

    PriorityCenterText(iClient, GetAdminImmunityLevel(GetUserAdmin(iAuthor), "My message's priority depends on my immunity level.");

    IF old priority == new priority THEN old message is overwritten by new message.

*/
stock PriorityCenterText(iClient, iPriority = MIN_INT, const String:szFormat[], any:...)
{
    if (iClient <= 0 || iClient > MaxClients)
    {
        ThrowError("Invalid client index %i", iClient);
    }
    
    if (!IsClientInGame(iClient))
    {
        ThrowError("Client %i is not in game", iClient);
    }

    if (s_iLastPriority[iClient] > iPriority)
    {
        if (GetEngineTime() >= s_flResetPriority[iClient])
        {
            s_iLastPriority[iClient] = MIN_INT;
        }
        else
        {
            return;
        }
    }

    if (iPriority > s_iLastPriority[iClient])
    {
        s_flResetPriority[iClient] = GetEngineTime() + 5.0;
        s_iLastPriority[iClient] = iPriority;
    }

    decl String:szBuffer[MAX_CENTER_TEXT];
    SetGlobalTransTarget(iClient);
    VFormat(szBuffer, sizeof(szBuffer), szFormat, 4);
    PrintCenterText(iClient, "%s", szBuffer);
}

/*
    Send priority center text to everyone.
    This will obey priority sent to via PriorityCenterText() and not overwrite if it's lower priority

    PriorityCenterTextAll(_, "Message")

    Is the proper way to send something that can be overwritten by everything.
*/
stock PriorityCenterTextAll(iPriority = MIN_INT, const String:szFormat[], any:...)
{
    decl String:szBuffer[MAX_CENTER_TEXT];

    for (new i = 1; i <= MaxClients; i++)
    {
        if (IsClientInGame(i))
        {
            SetGlobalTransTarget(i);
            VFormat(szBuffer, sizeof(szBuffer), szFormat, 3);
            PriorityCenterText(i, iPriority, "%s", szBuffer);
        }
    }
}

/*
    Send priority center text to everyone.
    This version bypasses the priority in PriorityCenterText() with its own internal counter.

    This version will ALWAYS have higher priority than the functions above, so long as it has higher priority than 'itself'

    The priority of all players will be completely maxed out to achieve this.
*/
stock PriorityCenterTextAllEx(iPriority = MIN_INT+1, const String:szFormat[], any:...)
{
    if (iPriority == MIN_INT)
    {
        iPriority++;
    }
    
    if (s_iLastPriority[0] > iPriority)
    {
        if (GetEngineTime() >= s_flResetPriority[0])
        {
            s_iLastPriority[0] = MIN_INT;

            for (new i = 1; i <= MaxClients; i++)
            {
                s_iLastPriority[i] = MIN_INT;
            }
        }
        else
        {
            return;
        }
    }

    if (iPriority > s_iLastPriority[0])
    {
        s_flResetPriority[0] = GetEngineTime() + 5.0;

        s_iLastPriority[0] = iPriority;

        for (new i = 1; i <= MaxClients; i++)
        {
            s_iLastPriority[i] = MAX_INT;
        }
    }

    decl String:szBuffer[MAX_CENTER_TEXT];

    for (new i = 1; i <= MaxClients; i++)
    {
        if (IsClientInGame(i))
        {
            SetGlobalTransTarget(i);
            VFormat(szBuffer, sizeof(szBuffer), szFormat, 3);
            PrintCenterText(i, "%s", szBuffer);
        }
    }
}
