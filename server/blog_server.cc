#include "httplib.h"
#include "db.hpp"



int main()
{
  using namespace httplib;
  using namespace blog_system;

  static MYSQL* mysql = NULL;
  mysql = blog_system::MySQLInit();

  BlogTable blog_table(mysql);
  TagTable tag_table(mysql);


  Server server;

  server.Post("/blog", [&blog_table](const Request& req, Response& resp){
        printf("新增博客! \n");

        // 解析Post中的body
        Json::Reader reader;
        Json::FastWriter writer;
        Json::Value req_json;
        Json::Value resp_json;

        bool ret = reader.parse(req.body, req_json);
        if(!ret){
          printf("解析失败！%s\n", req.body.c_str());

          // 构造对象 返回响应
          resp_json["ok"] = false;
          resp_json["reason"] = "input data parse falied";
          resp.status = 400;
          resp.set_content(writer.write(resp_json), "application/json");
          return;
        }

        // 校验数据
        if(req_json["title"].empty() || req_json["content"].empty()
            || req_json["tag_id"].empty() || req_json["create_time"].empty()){
          printf("请求数据格式错误! %s\n", req.body.c_str());

          resp_json["ok"] = false;
          resp_json["reason"] = "input data farmat error";
          resp.status = 400;
          resp.set_content(writer.write(resp_json), "application/json");
          return;
        }

        // 调用接口
        ret = blog_table.Insert(req_json);
        if(!ret){
          printf("博客插入失败! \n");
          resp_json["ok"] = false;
          resp_json["reason"] = "blog insert failed";
          resp.status = 500;
          resp.set_content(writer.write(resp_json), "application/json");
          return;
        }

        // 构造正确响应
        printf("博客插入成功!\n");
        resp_json["ok"] = true;
        resp.set_content(writer.write(resp_json), "application/json");
        return;
      });


  return 0;
}
