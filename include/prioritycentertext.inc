#if defined __prioritycentertext_0_included
 #endinput
#endif
#define __prioritycentertext_0_included


/*
    PriorityCenterText (Version 0x02)

    Only one message can be shown in center text at a time.
    These stocks allow that space to be given different priority levels that prevent new messages from overwriting what's there.

    By: Chdata

*/
static s_iLastPriority[MAXPLAYERS+1] = {-1,...};
static s_iLastUserId[MAXPLAYERS+1] = {0,...};

/*
    An example of how to use this:

    PriorityCenterText(iClient, _, "This message has no priority and will be overwritten by anything else.");
    PriorityCenterText(iClient, GetAdminImmunityLevel(GetUserAdmin(iAuthor), "My message's priority depends on the author's admin immunity level.");

    IF old priority == new priority THEN old message is overwritten by new message.

*/
stock PriorityCenterText(iClient, iPriority = -1, const String:szFormat[], any:...)
{
    if (iPriority > s_iLastPriority[iClient])
    {
        new Handle:hData;
        CreateDataTimer(5.0, RevertPriorityCenterText, hData, TIMER_FLAG_NO_MAPCHANGE); // 5.0 is the default duration center text stays for
        WritePackCell(hData, GetClientUserId(iClient));
        WritePackCell(hData, s_iLastPriority[iClient]);
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
    PriorityCenterTextAll(_, "This message is suitable for being redisplayed in a timer, for example to show a boss's HP.");

    PriorityCenterTextAll(1, "This message won't be overwritten by the message above until 5 seconds pass. Good for one time events.");
*/
stock PriorityCenterTextAll(iPriority = -1, const String:szFormat[], any:...)
{
    if (iPriority > s_iLastPriority[0])
    {
        new Handle:hData;
        CreateDataTimer(5.0, RevertPriorityCenterText, hData, TIMER_FLAG_NO_MAPCHANGE); // 5.0 is the default duration center text stays for
        WritePackCell(hData, -1);
        WritePackCell(hData, s_iLastPriority[0]);

        for (new i = 0; i <= MaxClients; i++) // Our loop includes [0] (console)
        {
            s_iLastPriority[i] = iPriority;
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
            //PriorityCenterText(i, iPriority, "%s", szBuffer);
        }
    }
}

public Action:RevertPriorityCenterText(Handle:hTimer, Handle:hData)
{
    ResetPack(hData);
    new iUserId = ReadPackCell(hData);
    new iPriority = ReadPackCell(hData);
    new iClient = (iUserId != -1) ? GetClientOfUserId(iUserId) : -1;

    if (iClient == -1) // "All"
    {
        for (new i = 0; i <= MaxClients; i++)
        {
            s_iLastPriority[i] = iPriority;
        }
    }
    else
    {
        if (iClient && IsClientInGame(iClient))
        {
            new bool:bRevert = true;
            if (iUserId != s_iLastUserId[iClient])  // This is so we don't need to s_iLastPriority[iClient] = -1 during OnClientDisconnect
            {
                bRevert = !s_iLastUserId[iClient];  // If it was still 0 (unintialized), then we still revert.
                s_iLastUserId[iClient] = iUserId;   // Otherwise, if a client ent index had its UserId changed, it means someone reconnected into someone elses spot, so we need to reset them.
            }

            if (bRevert)
            {
                s_iLastPriority[iClient] = iPriority;
            }
            else
            {
                s_iLastPriority[iClient] = -1;      // This happens when the UserId of a client ent index has changed, to reset them.
            }
        }
    }
}
