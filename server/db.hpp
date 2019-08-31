#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <mysql/mysql.h>
#include <jsoncpp/json/json.h>



namespace blog_system{

  MYSQL* MySQLInit(){
    MYSQL* connect_fd = mysql_init(NULL);

    if(mysql_real_connect(connect_fd, "127.0.0.1", "root", "74110jiuwangjihB", "blog_system", 9999, NULL, 0) == NULL){
      printf("连接失败:%s\n", mysql_error(connect_fd));
      return NULL;
    }

    mysql_set_character_set(connect_fd, "utf8");
    return connect_fd;
  }

  void MySQLRelease(MYSQL* connect_fd){
    mysql_close(connect_fd);
  }

  class BlogTable{
    public:
      BlogTable(MYSQL* connect_fd):_mysql(connect_fd){}

      bool Insert(Json::Value& blog){
        const std::string& content = blog["content"].asString();
        std::unique_ptr<char> to(new char[content.size() * 2 + 1]);
        mysql_real_escape_string(_mysql, to.get(), content.c_str(),content.size());
        
        // 拼接
        std::unique_ptr<char> sql(new char[content.size() * 2 + 1024 * 1024]);
        sprintf(sql.get(), "insert into blog_table values(null, '%s', '%s', %d, '%s')", 
            blog["title"].asCString(),
            to.get(),
            blog["tag_id"].asInt(),
            blog["create_time"].asCString()
            );

        // 执行sql语句
        int ret = mysql_query(_mysql, sql.get());
        if(ret != 0){
          printf("插入博客失败! %s\n", mysql_error(_mysql));
          return false;
        }

        printf("执行插入博客成功! \n");
        return true;
      }

      bool SelectAll(Json::Value *blogs, const std::string& tag_id =""){
       char sql[1024 * 1024] = {0};
       if(tag_id == ""){
          sprintf(sql, "select blog_id, title, tag_id, create_time form blog_table");
       }else{
         sprintf(sql, "select blog_id, title, tag_id, create_time form blog_table where tag_id == %d", std::stoi(tag_id));
       }
       
       int ret = mysql_query(_mysql, sql);
       if(ret != 0){
         printf("执行查找博客失败 %s\n", mysql_error(_mysql));
         return false;
       }
       printf("执行查找博客成功!\n");

       MYSQL_RES* result = mysql_store_result(_mysql);
       int rows = mysql_num_rows(result);
       for(int i = 0; i < rows; ++i){
         MYSQL_ROW row = mysql_fetch_row(result);
         Json::Value blog;
         blog["blog_id"] = atoi(row[0]);
         blog["title"] = row[1];
         blog["tag_id"] = atoi(row[2]);
         blog["create_time"] = row[3];
         blogs->append(blog);
       }

       mysql_free_result(result);
       printf("执行查找博客成功!\n 共查找到%d条博客\n",rows);
       return true;
      }

      bool SelectOne(int blog_id, Json::Value * blog){
        char sql[1024 * 1024] = {0};
        sprintf(sql, "select blog_id, title, tag_id, create_time form blog_table where blog_id == %d", blog_id);
        int ret = mysql_query(_mysql, sql);
        if(ret != 0){
          printf("执行特定查找博客失败！%s\n", mysql_error(_mysql));
          return false;
        }
        printf("执行查找博客成功!\n");
        MYSQL_RES * result = mysql_store_result(_mysql);
        int rows = mysql_num_rows(result);
        if(rows != 1){
          printf("查找到的博客不止一条 有%d条\n", rows);
          return false;
        }
        MYSQL_ROW row = mysql_fetch_row(result);
        (*blog)["blog_id"] = atoi(row[0]);
        (*blog)["title"] = row[1];
        (*blog)["tag_id"] = row[2];
        (*blog)["create_time"] = row[3];

        return true;
      }

      bool Update(const Json::Value& blog){
        const std::string& content = blog["content"].asString();
        std::unique_ptr<char> to(new char[content.size() * 2 + 1]);
        mysql_real_escape_string(_mysql, to.get(), content.c_str(), content.size());

        std::unique_ptr<char> sql(new char[content.size() * 2 + 1024 * 1024]);
        sprintf(sql.get(), "update blog_table set title = '%s', content = '%s', tag_id = %d where blog_id = %d",
            blog["title"].asCString(),
            to.get(),
            blog["tag_id"].asInt(),
            blog["blog_id"].asInt()
            );

        int ret = mysql_query(_mysql, sql.get());
        if(ret != 0){
          printf("执行更新博客失败！%s\n", mysql_error(_mysql));
          return false;
        }

        printf("执行更新博客成功！\n");
        return true;

      }

      bool Delete(int blog_id){
        char sql[1024 * 1024] = {0};
        sprintf(sql, "delete from blog_table where blog_id = %d", blog_id);
        int ret = mysql_query(_mysql, sql);
        if(ret != 0){
          printf("执行删除博客失败! %s\n", mysql_error(_mysql));
          return false;
        }

        printf("执行删除博客成功！\n");
        return true;
      }

    private:
      MYSQL* _mysql;
  };

  class TagTable{
    public:
      TagTable(MYSQL* connnect_fd):_mysql(connnect_fd){}
      
      bool Insert(const Json::Value& tag){
        char sql[1024 * 1024] = {0};
        sprintf(sql, "insert into tag_table values(null, '%s')",
        tag["tag_name"].asCString());

        int ret = mysql_query(_mysql, sql);
        if(ret != 0){
          printf("插入标签失败! %s\n", mysql_error(_mysql));
          return false;
        }
        printf("插入标签成功!\n");
        return true;
      }

      bool Delete(int tag_id){
        char sql[1024 * 1024] = {0};
        sprintf(sql, "delete form table where tag_id = %d", tag_id);
        int ret = mysql_query(_mysql, sql);
        if(ret != 0){
          printf("删除标签失败! %s\n", mysql_error(_mysql));
          return false;
        }

        printf("删除标签成功!\n");
        return true;
      }

      bool SelectAll(Json::Value *tags){
        char sql[1024 * 1024] = {0};
        sprintf(sql, "select tag_id, tag_name from tag_table");

        int ret = mysql_query(_mysql, sql);
        if(ret != 0){
          printf("查找标签失败! %s\n", mysql_error(_mysql));
          return false;
        }

        MYSQL_RES * result = mysql_store_result(_mysql);
        int rows = mysql_num_rows(result);
        for(int i = 0; i < rows; i++){
          MYSQL_ROW row = mysql_fetch_row(result);
          Json::Value tag;
          tag["tag_id"] = atoi(row[0]);
          tag["tag_name"] = row[1];
          tags->append(tag);
        }
        mysql_free_result(result);
        printf("查找标签成功!\n");
        return true;
      }

    private:
      MYSQL* _mysql;
  };






} // end of blog_system
