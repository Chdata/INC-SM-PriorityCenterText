#if defined __prioritycentertext_0_included
 #endinput
#endif
#define __prioritycentertext_0_included


/*
    PriorityCenterText (Version 0x03)

    Only one message can be shown in center text at a time.
    These stocks allow that space to be given different priority levels that prevent new messages from overwriting what's there.

    By: Chdata

*/
static s_iLastPriority[TF_MAX_PLAYERS] = {-1,...};
static s_iLastUserId[TF_MAX_PLAYERS] = {0,...};
static Handle:s_hPCTStack[TF_MAX_PLAYERS] = {INVALID_HANDLE,...};

/*
    An example of how to use this:

    PriorityCenterText(iClient, GetAdminImmunityLevel(GetUserAdmin(iAuthor), "My message's priority depends on my immunity level.");

    IF old priority == new priority THEN old message is overwritten by new message.

*/
stock PriorityCenterText(iClient, iPriority = -1, const String:szFormat[], any:...)
{
    if (iClient <= 0 || iClient > MaxClients)
    {
        ThrowError("Invalid client index %i", iClient);
    }
    
    if (!IsClientInGame(iClient))
    {
        ThrowError("Client %i is not in game", iClient);
    }

    PCTCheck();

    if (iPriority > s_iLastPriority[iClient])
    {
        CreateTimer(5.0, RevertPriorityCenterText, GetClientUserId(iClient), TIMER_FLAG_NO_MAPCHANGE);
        PushStackCell(s_hPCTStack[iClient], s_iLastPriority[iClient]);
        s_iLastPriority[iClient] = iPriority;
    }

    if (s_iLastPriority[iClient] > iPriority)
    {
        return;
    }

    decl String:szBuffer[MAX_CENTER_TEXT];
    SetGlobalTransTarget(iClient);
    VFormat(szBuffer, sizeof(szBuffer), szFormat, 4);
    PrintCenterText(iClient, "%s", szBuffer);
}

/*
    Send priority center text to everyone.
    This will obey priority sent to via PriorityCenterText() and not overwrite if it's lower priority
*/
stock PriorityCenterTextAll(iPriority = -1, const String:szFormat[], any:...)
{
    PCTCheck();

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

    The priority of all players will be incremented by one to achieve this.

    TODO: PCTGetPriority(iClient)
*/
stock PriorityCenterTextAllEx(iPriority = -1, const String:szFormat[], any:...)
{
    PCTCheck();

    if (iPriority > s_iLastPriority[0])
    {
        CreateTimer(5.0, RevertPriorityCenterText, -1, TIMER_FLAG_NO_MAPCHANGE);

        PushStackCell(s_hPCTStack[0], s_iLastPriority[0]);
        s_iLastPriority[0] = iPriority;

        for (new i = 1; i <= MaxClients; i++) // Our loop includes [0] (console)
        {
            PushStackCell(s_hPCTStack[i], s_iLastPriority[i]);
            s_iLastPriority[i]++;
        }
    }

    if (s_iLastPriority[0] > iPriority)
    {
        return;
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

public Action:RevertPriorityCenterText(Handle:hTimer, any:UserId)
{
    new iClient = (UserId != -1) ? GetClientOfUserId(UserId) : -1;  // TODO: pass any:Client and use some IsClientConnected / UserId hax to determine actual disconnects?

    if (iClient == -1) // "All"
    {
        for (new i = 0; i <= MaxClients; i++)
        {
            PopStackCell(s_hPCTStack[i], s_iLastPriority[i]);
        }
    }
    else
    {
        if (iClient && IsClientInGame(iClient))
        {
            new bool:bRevert = true;
            if (UserId != s_iLastUserId[iClient])  // This is so we don't need to s_iLastPriority[iClient] = -1 during OnClientDisconnect
            {
                bRevert = !s_iLastUserId[iClient];  // If it was still 0 (unintialized), then we still revert.
                s_iLastUserId[iClient] = UserId;   // Otherwise, if a client ent index had its UserId changed, it means someone reconnected into someone elses spot, so we need to reset them.
            }

            if (bRevert)
            {
                PopStackCell(s_hPCTStack[iClient], s_iLastPriority[iClient]);
            }
            else
            {
                while(PopStack(s_hPCTStack[iClient])){} // Empty out this stack
                s_iLastPriority[iClient] = -1;          // This happens when the UserId of a client ent index has changed, to reset them.
            }
        }
    }
}

stock PCTCheck()
{
    if (s_hPCTStack[0] == INVALID_HANDLE)
    {
        for (new i = 0; i <= MaxClients; i++)
        {
            s_hPCTStack[i] = CreateStack();
        }
    }
}
