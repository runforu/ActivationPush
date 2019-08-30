
#include <process.h>
#include <stdio.h>
#include "HttpPost.h"
#include "Loger.h"
#include "Processor.h"
#include "ServerApi.h"

void Processor::Shutdown(void) {
    HttpPost::Instance().Stop();
    ShowStatus();

#ifdef _LICENSE_VERIFICATION_
    LicenseService::Instance().Stop();
#endif  // !_LICENSE_VERIFICATION_
}

Processor::Processor() : m_reinitialize_flag(0), m_disable_plugin(0) {}

Processor& Processor::Instance() {
    static Processor _instance;
    return _instance;
}

void Processor::ShowStatus() {
    LOG("OrderMonitor is going to shutdown.");
}

void Processor::Initialize() {
    FUNC_WARDER;

    Config::Instance().GetInteger("Disable Plugin", &m_disable_plugin, "0");
    Config::Instance().GetString("Server", m_notice_server, sizeof(m_notice_server) - 1, "http://localhost");
    HttpPost::Instance().SetUrl(m_notice_server);

#ifdef _LICENSE_VERIFICATION_
    LicenseService::Instance().ResetLicense();
#endif  // !_LICENSE_VERIFICATION_
}

void Processor::OnStopoutsApply(const UserInfo* user, const ConGroup* group, const ConSymbol* symbol, TradeRecord* stopout) {
    if (!CommonCheck()) {
        return;
    }

    boost::property_tree::ptree notice;
    notice.put("user", user->login);
    notice.put("order", stopout->order);
    notice.put("mode", "STOPOUT");
    HttpPost::Instance().PostNotice(notice);
}

void Processor::OnStopsApply(const UserInfo* user, const ConGroup* group, const ConSymbol* symbol, TradeRecord* trade,
                             const int isTP) {
    if (!CommonCheck()) {
        return;
    }

    boost::property_tree::ptree notice;
    notice.put("user", user->login);
    notice.put("order", trade->order);
    notice.put("mode", isTP ? "TP" : "SL");
    HttpPost::Instance().PostNotice(notice);
}

void Processor::OnPendingsApply(const UserInfo* user, const ConGroup* group, const ConSymbol* symbol,
                                const TradeRecord* pending, TradeRecord* trade) {
    if (!CommonCheck()) {
        return;
    }

    boost::property_tree::ptree notice;
    notice.put("user", user->login);
    notice.put("order", pending->order);
    notice.put("mode", "ACTIVATION");
    HttpPost::Instance().PostNotice(notice);
}
