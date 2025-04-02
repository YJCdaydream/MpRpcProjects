#include "test.pb.h"
#include <iostream>


using namespace fixbug;

int main()
{
    
    LoginResponse req;
    ResultCode * ems =  req.mutable_result();
    ems->set_errmsg("error");
    ems->set_errcode(1);

    
    GetFriendListResponse rflp;
    User * friendlist = rflp.add_friend_list();
    friendlist->set_age(22);
    friendlist->set_name("zhang san");
    friendlist->set_sex(User::MAN);

    std::cout<< rflp.friend_list_size()<<std::endl;
    return 0;
}