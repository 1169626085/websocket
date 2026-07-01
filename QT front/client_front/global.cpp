#include "global.h"


std::function<void(QWidget*)> repolish=[](QWidget* w){
    w->style()->unpolish(w);
    w->style()->polish(w);
};

SearchInfo::SearchInfo()
    : _uid(0)
    , _sex(0)
    , _isFriend(false)
{
}

SearchInfo::SearchInfo(int uid, const QString& name, const QString& email, const QString& nick,
                       const QString& desc, int sex, const QString& icon, bool isFriend)
    : _uid(uid)
    , _name(name)
    , _email(email)
    , _nick(nick)
    , _desc(desc)
    , _sex(sex)
    , _icon(icon)
    , _isFriend(isFriend)
{
}

FriendApplyInfo::FriendApplyInfo()
    : _fromUid(0)
    , _toUid(0)
{
}

FriendApplyInfo::FriendApplyInfo(int fromUid, int toUid, const QString& fromName, const QString& message)
    : _fromUid(fromUid)
    , _toUid(toUid)
    , _fromName(fromName)
    , _message(message)
{
}
