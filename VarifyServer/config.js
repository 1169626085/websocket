const fs=require('fs');

let config=JSON.parse(fs.readFileSync('config.json','utf8'));
let email_user = config.email?.user || config.email_user;
let email_pass = config.email?.pass || config.email_pass;
let mysql_host = config.mysql?.host || "127.0.0.1";
let mysql_port = config.mysql?.port || 3306;
let redis_host = config.redis?.host || "127.0.0.1";
let redis_port = config.redis?.port || 6380;
let redis_passwd = config.redis?.passwd || "";
let code_prefix = "code_";
module.exports={email_pass,email_user,mysql_host,mysql_port,redis_host,redis_port,redis_passwd,code_prefix}
