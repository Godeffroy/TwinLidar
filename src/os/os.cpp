#include "os.h"
#include "console.h"

OS OS::m_instance;

void OS::run(){
    switch(m_state){
        case BOOT:
            boot_routine();
            break;
        case RUNNING:
            run_routine();
            break;
        default: 
        break;
    }
}

void OS::start(){
    m_state = RUNNING;
}

void OS::boot_routine(){
    executeRoutine(m_bootRoutine);
    start();
}

void OS::run_routine(){
    updateServices();
    executeRoutine(m_runRoutine);
    if(currentJob() != nullptr){
        if(currentJob()->isCompleted() || currentJob()->isCancelled()) killCurrentJob();
        else currentJob()->run();
    }
}

void OS::setRountine(SystemState state, routine_ptr func_ptr){
    if(func_ptr == nullptr) return;
    switch(state){
        case BOOT:
            m_bootRoutine = func_ptr;
            break;
        case RUNNING:
            m_runRoutine = func_ptr;
            break;
        default:
        break;
    }
}

void OS::setState(SystemState state){
    m_state = state;
}

void OS::attachService(Service* service){
    if(service != nullptr){
        m_services[service->id()] = service;
        Console::trace() << "Attached service: " << service->id() << Console::endl;
        service->enable();
        service->onAttach();
    }
}

bool OS::hasService(ServiceID s) const{
    return m_services.find(s) != m_services.end();
}

bool OS::statusService(ServiceID serviceID) const{
    if(hasService(serviceID))
        return m_services.at(serviceID)->enabled();
    else
        return false;
}

bool OS::debug(ServiceID s){
    if(hasService(s))
        return m_services[s]->debug();
    else
        return false;
}

void OS::toggleDebug(ServiceID s){
    if(hasService(s))
        m_services[s]->toggleDebug();
}

void OS::wait(unsigned long time, bool async) {
    m_timer.setDuration(time);
    m_timer.start();
    addJob(&m_timer);
    if(!async)while(m_timer.isPending())run();
}

void OS::waitUntil(Job& obj, bool async){
    addJob(&obj);
    if(!async) while(obj.isPending()) run();
}

void OS::execute(Job& obj, bool async){
    addJob(&obj);
    if(!async)while(obj.isPending()) run();
}

void OS::flush(){
    while(isBusy()) run();
}

bool OS::isBusy() {
    return m_jobs.size() != 0;
};

Job* OS::currentJob(){
    if(m_jobs.size() == 0) return nullptr;
    else return m_jobs.front();
}
void OS::addJob(Job* job){
    m_jobs.push(job);
}
void OS::killCurrentJob(){
    if(m_jobs.size() == 0) return;
    m_jobs.pop();
}

void OS::updateServices(){
    for(const auto& service : m_services) {
        if(service.second->enabled()) {
            service.second->onUpdate();
        }
    }
}

void OS::enable(ServiceID id){
    if(hasService(id)) m_services[id]->enable();
}

void OS::disable(ServiceID id){
    if(hasService(id)) m_services[id]->disable();
}

void OS::executeRoutine(routine_ptr routine){
    if(routine != nullptr){
        routine();
    }else{
        Console::error("OS") << "Routine is nullptr" << Console::endl;
    }
}
