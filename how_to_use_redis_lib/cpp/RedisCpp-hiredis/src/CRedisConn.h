#pragma once

#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include <string>
#include <list>
#include <map>
#include <stdexcept>

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "hiredis.lib")

#include <winsock.h>
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4200)
#include <hiredis.h>
#pragma warning(pop)
#else
#include <hiredis.h>
#endif

#ifndef DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN( typeName )	\
	typeName( const typeName & );		\
	typeName & operator=( const typeName & )
#endif

namespace RedisCpp
{

typedef std::list<std::string> ValueList;
typedef std::map<std::string , std::string> ValueMap;



///< 지정된 요소 앞이나 지정된 요소 뒤에 삽입
/*typedef */enum INSERT_POS
{
	BEFORE,			///< 지정된 요소 앞에 삽입
	AFTER
} E_INSERT_POS;

/**
 *@brief 이 클래스는 hiredis를 기반으로 하며 redis-server에 대한 링크를 유지하는데 사용된다.
 * 
 * 샘플 코드:
 *int main( )
 {
 RedisCpp::RedisConn con;
 if ( !con.connect( "127.0.0.1", 6379, "521110", 5 ) )
 {
 std::cout << "connect error " << con.getErrorStr( ) << std::endl;
 return 0;
 }

 std::string value;
 if ( !con.hget( "newHash", "yuhaiyang", value ) )
 {
 std::cout << "hget error " << con.getErrorStr( ) << std::endl;
 }
 return 0;
 }
 *
 */
class CRedisConn
{
public:
	CRedisConn( void )
	{
		_redCtx = NULL;
		_host.clear();
		_password.clear();
		_port = 0;
		_timeout = 0;
		_connected = false;
		_errStr = _errDes[ERR_NO_ERROR];
	}

	virtual ~CRedisConn(void)
	{
		disConnect();
	}

	/**
	 *@brief 연결 정보 초기화
	 *
	 *연결을 초기화 하는데 사용 되는 세부 사항. connect() 전에 사용한다. 직접 호출 가능
	 *bool connect( const std::string &host ,const uint16_t port, const std::string& password , const uint32_t timeout );
	 */
	void init( const std::string &host = "127.0.0.1" , const uint16_t port = 6379 ,
	                const std::string& password = "" , const uint32_t timeout = 0 )
	{
		_host = host;
		_port = port;
		_password = password;
		_timeout = timeout;
	}


	/**
	 *@brief 접속　redis 데이터베이스
	 *
	 * 이 메소드는 데이터베이스를 연결하기 위해 init() 후에 호출한다.
	 *
	 *@return 성공　true, 실패　false
	 */
	bool connect()
	{
		if (_connected)
		{
			disConnect();
		}

		struct timeval timeoutVal;
		timeoutVal.tv_sec = _timeout;
		timeoutVal.tv_usec = 0;

		_redCtx = redisConnectWithTimeout(_host.c_str(), _port, timeoutVal);
		if (_getError(_redCtx))
		{
			if (NULL != _redCtx)
			{
				redisFree(_redCtx);
				_redCtx = NULL;
			}
			_connected = false;
			return false;
		}

		_connected = true;
		return true;

		// if connection  need password
	/*
		if ( _password == "" )
		{
			return true;
		}
		else
		{
			return ( auth( _password ) );   //< 승인 실패도 false 반환
		}
	*/
	}

	/**
	 *@brief 연결 끊기. Redis 인 메모리 데이터베이스
	 */
	void disConnect( )
	{
		if (_connected && NULL != _redCtx)
		{
			redisFree(_redCtx);
			_redCtx = NULL;
		}
		_connected = false;
	}

	/**
	 *@brief redis 데이터 베이스를 연결한 후 인증
	 *
	 *@param [in] 로그인 비밀 번호
	 *@return 성공　true, 실패　false.
	 */
	bool auth( const std::string& password )
	{
		if (!_connected)
		{
			_errStr = _errDes[ERR_NO_CONNECT];
			return false;
		}

		bool ret = false;
		_password = password;
		redisReply* reply = static_cast<redisReply*>(redisCommand(_redCtx, "AUTH %s",
			_password.c_str()));

		if (_getError(reply))
		{
			ret = false;
		}
		else
		{
			ret = true;
		}

		if (NULL != reply)
		{
			freeReplyObject(reply);
		}
		return ret;
	}

	/**
	 *@brief 접속
	 *
	 * 초기화하는데 필요한 정보. 비밀번호가 비어 있지 않으면 인증 자동으로 수행된다.
	 *
	 *@param [in]  host .  redis-server의 ip。
	 *@param [in] port .redis-server의 port 。
	 *@param [in] password . redis-server의 비밀번호. 이 매개 변수는 비워 둘 수 있으며, 입력하지 않으면 인증을 시작하지 않는다.
	 *@param [in] timeout .연결 시간 초과
	 *@return 성공 true，　실패 false
	 */
	bool connect( const std::string &host , const uint16_t port , const std::string& password =
	                "" , const uint32_t timeout = 0 )
	{
		// Init attribute.
		init(host, port, password, timeout);

		return (connect());
	}

	/**
	 * @brief 연결 상태 조사
	 *
	 * @return 연결 되어 있다면 true를 반환하고, 그렇지 않으면 false를 반환.
	 */
	bool ping( )
	{
		if (!_connected || !_redCtx)
		{
			_errStr = _errDes[ERR_NO_CONNECT];
			return false;
		}

		redisReply* reply = static_cast<redisReply*>(redisCommand(_redCtx, "PING"));
		bool ret = false;

		if (_getError(reply))
		{
			ret = false;
		}
		else
		{
			ret = true;
		}

		if (NULL != reply)
		{
			freeReplyObject(reply);
		}

		return ret;
	}

	/**
	 *@brief redis에 다시 연결
	 *
	 *@return 재 연결 성공　true, 재 연결 실패　false.
	 */
	bool reconnect( )
	{
		return (connect());
	}

	/**
	 * @brief redis와 연결 여부
	 *
	 * @return 연결 되어 있다면　true, 아니면　false.
	 */
	inline bool isConneced( )
	{
		return _connected;
	}

	/**
	 * @brief 오류 원인
	 *
	 * ＠return 에러 원인 반환
	 */
	const std::string getErrorStr() const
	{
		return _errStr;
	}

	/**
	 * @brief 명령 매개 변수를 redis 서버로 직접 보낸다.
	 *
	 * ＠param [in] format 
	 * ＠param [in] ... 명령 및 데이터 문자열
	 * @return NULL, redis 실행 실패 및 연결이 끊아진 경우. 성공이라면　redisReply 반환
	 */
	redisReply* redisCmd( const char *format , ... )
	{
		va_list ap;
		va_start(ap, format);
		redisReply* reply = static_cast<redisReply*>(redisvCommand(_redCtx, format, ap));
		va_end(ap);
		return reply;
	}

	///////////////////////////////// list 사용법 /////////////////////////////////////
	/**
	 * @brief list 왼쪽에 삽입
	 * @param [in] retval 삽입 성공 후 list 길이
	 * @return 실패 false，성공 true
	 */
	bool lpush( const std::string& key , const std::string& value , uint64_t& retval )
	{
		if (!_connected || !_redCtx)
		{
			_errStr = _errDes[ERR_NO_CONNECT];
			return false;
		}

		retval = 0;
		bool ret = false;

		redisReply* reply = redisCmd("LPUSH %s %s", key.c_str(), value.c_str());

		if (_getError(reply))
		{
			ret = false;
		}
		else
		{
			retval = reply->integer;
			ret = true;
		}

		if (NULL != reply)
		{
			freeReplyObject(reply);
		}
		return ret;

	}

	/**
	 * @brief list 왼쪽에서 추출
	 * @param [in] value 빼낸 값
	 * @return 실패 false，성공 true
	 */
	bool lpop( const std::string& key , std::string& value )
	{
		if (!_connected || !_redCtx)
		{
			_errStr = _errDes[ERR_NO_CONNECT];
			return false;
		}

		bool ret = false;
		redisReply* reply = redisCmd("LPOP %s", key.c_str());

		if (_getError(reply))
		{
			ret = false;
		}
		else
		{
			// 실패
			if (NULL == reply->str)
			{
				_errStr = _errDes[ERR_NO_KEY];
				value = "";
				ret = false;
			}
			else
			{
				value = reply->str;
				ret = true;
			}
		}

		if (NULL != reply)
		{
			freeReplyObject(reply);
		}

		return ret;
	}

	/**
	 * @brief 지정된 범위의 list에 있는 요소를 가져온다.
	 * @param [in] start 간격 시작 색인，stop 간격 끝 색인, valueList 간격 list
	 */
	bool lrange( const std::string &key , uint32_t start , int32_t end, ValueList& valueList )
	{
		if (!_connected || !_redCtx)
		{
			_errStr = _errDes[ERR_NO_CONNECT];
			return false;
		}

		bool ret = false;
		redisReply* reply = redisCmd("LRANGE %s %d %d", key.c_str(), start, end);

		if (_getError(reply))
		{
			ret = false;
		}
		else
		{
			if (REDIS_REPLY_ARRAY == reply->type && 0 == reply->elements) //<  key는 list 열거 타입 start > end
			{
				_errStr = std::string(_errDes[ERR_INDEX]) + " or "
					+ _errDes[ERR_NO_KEY];
				ret = false;

			}
			else
			{
				_getArryToList(reply, valueList);
				ret = true;
			}
		}

		if (NULL != reply)
		{
			freeReplyObject(reply);
		}

		return ret;
	}

	/**
	 * @brief list 오른쪽에 삽입
	 * @param [in] retval 삽입 성공 후의 list 요소 수
	 * @return 실패 false，성공 true
	 */
	bool rpush(const std::string& key, const std::string& value, uint64_t& retval)
	{
		if (!_connected || !_redCtx)
		{
			_errStr = _errDes[ERR_NO_CONNECT];
			return false;
		}

		retval = 0;
		bool ret = false;

		redisReply* reply = redisCmd("RPUSH %s %s", key.c_str(), value.c_str());

		if (_getError(reply))
		{
			ret = false;
		}
		else
		{
			retval = reply->integer;
			ret = true;
		}

		if (NULL != reply)
		{
			freeReplyObject(reply);
		}

		return ret;
	}

	/**
	 * @brief list 오른쪽에서 빼내기
	 * @param [in] value 빼낸 값
	 * @return 실패 false，성공 true
	 */
	bool rpop( const std::string& key , std::string& value )
	{
		if (!_connected || !_redCtx)
		{
			_errStr = _errDes[ERR_NO_CONNECT];
			return false;
		}

		bool ret = false;
		redisReply* reply = redisCmd("RPOP %s", key.c_str());

		if (_getError(reply))
		{
			ret = false;
		}
		else
		{
			// 실패
			if (NULL == reply->str)
			{
				_errStr = _errDes[ERR_NO_KEY];
				value = "";
				ret = false;
			}
			else
			{
				value = reply->str;
				ret = true;
			}
		}

		if (NULL != reply)
		{
			freeReplyObject(reply);
		}

		return ret;
	}

	/**
	 * @brief LINSERT
	 * list의 특정 위치에 데이터 넣기
	 */
	bool linsert(const std::string& key, INSERT_POS position, const std::string& pivot,
		const std::string& value, int64_t& retval)
	{
		if (!_connected || !_redCtx)
		{
			_errStr = _errDes[ERR_NO_CONNECT];
			return false;
		}

		std::string pos;

		if (BEFORE == position)
		{
			pos = "BEFORE";
		}
		else if (AFTER == position)
		{
			pos = "AFTER";
		}

		bool ret = false;
		redisReply* reply = redisCmd("LINSERT %s %s %s %s", key.c_str(), pos.c_str(),
			pivot.c_str(), value.c_str());

		if (_getError(reply))	//< list 유형이 아님
		{
			ret = false;
		}
		else
		{
			if (REDIS_REPLY_INTEGER == reply->type)
			{
				if (reply->integer == -1)
				{
					_errStr = _errDes[ERR_NO_PIVOT];
					ret = false;
				}
				else if (reply->integer == 0)
				{
					_errStr = _errDes[ERR_NO_KEY];
					ret = false;
				}
				else
				{
					retval = reply->integer;
					ret = true;
				}
			}
			else
			{
				ret = false;
			}
		}

		if (NULL != reply)
		{
			freeReplyObject(reply);
		}

		return ret;
	}

	/**
	 * @brief LINDEX
	 * lindex key index
	 * index의 위치에 있는 데이터를 조회한다. 0 에서 시작한다
	 */
	bool lindex( const std::string& key , int32_t index , std::string& value )
	{
		if (!_connected || !_redCtx)
		{
			_errStr = _errDes[ERR_NO_CONNECT];
			return false;
		}

		bool ret = false;
		redisReply* reply = redisCmd("LINDEX %s %d", key.c_str(), index);

		if (_getError(reply))
		{
			ret = false;
		}
		else
		{
			// 실패
			if (REDIS_REPLY_NIL == reply->type)
			{
				_errStr = std::string(_errDes[ERR_NO_KEY]) + " or " +
					_errDes[ERR_INDEX];
				value = "";
				ret = false;
			}
			else
			{
				value = reply->str;
				ret = true;
			}
		}

		if (NULL != reply)
		{
			freeReplyObject(reply);
		}

		return ret;
	}

	/**
	 * @brief LLEN
	 * list에서 value의 개수를 조회 https://kwoncharlie.blog.me/220397447626
	 */
	bool llen( const std::string& key , uint64_t& retval )
	{
		if (!_connected || !_redCtx)
		{
			_errStr = _errDes[ERR_NO_CONNECT];
			return false;
		}

		bool ret = false;
		redisReply* reply = redisCmd("LLEN %s", key.c_str());

		if (_getError(reply))
		{
			ret = false;
		}
		else
		{
			if (REDIS_REPLY_INTEGER == reply->type && (0 == reply->integer))
			{
				_errStr = _errDes[ERR_NO_KEY];
				ret = false;
			}
			else
			{
				retval = reply->integer;
				ret = true;
			}
		}

		if (NULL != reply)
		{
			freeReplyObject(reply);
		}

		return ret;
	}




	//////////////////////////////  string 사용 방법 //////////////////////////////////////
	
	bool get(const std::string& key, std::string& value)
	{
		if (!_connected || !_redCtx)
		{
			_errStr = _errDes[ERR_NO_CONNECT];
			return false;
		}

		bool ret = false;
		redisReply* reply = redisCmd("GET %s", key.c_str());

		if (_getError(reply))
		{
			ret = false;
		}
		else
		{
			if (REDIS_REPLY_NIL == reply->type)
			{
				_errStr = std::string(_errDes[ERR_NO_KEY]) + " or " + 	_errDes[ERR_NO_FIELD];
				ret = false;
			}
			else
			{
				value = reply->str;
				ret = true;
			}
		}
		if (NULL != reply)
		{
			freeReplyObject(reply);
		}
		else
		{
		}

		return ret;
	}

	bool set(const std::string& key, const std::string& value, uint32_t& retval)
	{
		if (!_connected || !_redCtx)
		{
			_errStr = _errDes[ERR_NO_CONNECT];
			return false;
		}

		bool ret = false;
		redisReply* reply = redisCmd("SET %s %s", key.c_str(), value.c_str());

		if (_getError(reply))
		{
			ret = false;
		}
		else
		{
			retval = (uint32_t)reply->integer;
			ret = true;
		}

		if (NULL != reply)
		{
			freeReplyObject(reply);
		}

		return ret;
	}

	bool del(const std::string& key, uint32_t& retval)
	{
		if (!_connected || !_redCtx)
		{
			_errStr = _errDes[ERR_NO_CONNECT];
			return false;
		}

		bool ret = false;
		redisReply* reply = redisCmd("DEL %s", key.c_str());

		if (_getError(reply))
		{
			ret = false;
		}
		else
		{
			retval = (uint32_t)reply->integer;
			ret = true;
		}

		if (NULL != reply)
		{
			freeReplyObject(reply);
		}

		return ret;
	}


	//////////////////////////////   hash 사용 방법 //////////////////////////////////////

	/**
	 * @brief 해시 테이블에서 키와 필드에 해당하는 값을 가져온다.
	 * @param [in] key는 테이블 이름과 같은 키 이름이다
	 * @param [in] filed 필드 이름
	 * @param [out] value 얻은 값
	 * @return 성공 true，실패 false
	 * @warning 실패 값은 value""(string 초기화 값)
	 */
	bool hget(const std::string& key, const std::string& filed, std::string& value)
	{
		if (!_connected || !_redCtx)
		{
			_errStr = _errDes[ERR_NO_CONNECT];
			return false;
		}

		bool ret = false;
		redisReply* reply = redisCmd("HGET %s %s", key.c_str(), filed.c_str());

		if (_getError(reply))
		{
			ret = false;
		}
		else
		{
			if (REDIS_REPLY_NIL == reply->type)
			{
				_errStr = std::string(_errDes[ERR_NO_KEY]) + " or " +
					_errDes[ERR_NO_FIELD];
				ret = false;
			}
			else
			{
				value = reply->str;
				ret = true;
			}
		}
		if (NULL != reply)
		{
			freeReplyObject(reply);
		}
		else
		{

		}

		return ret;
	}
	

	/**
	 * @brief 해시 테이블에서 키와 필드의 값을 설정한다
	 * @param [in] key 테이블 이름과 같은 키 이름이다
	 * @param [in] filed 필드 이름
	 * @param [in] value 위의 두 매개 변수에 해당하는 값
	 * @param [out] retval 0:field가 존재하고 값을 덮어쓴다； 1： field가 존재하지 않고 새 필드가 생성되고 값이 설정 된다.
	 * @return 성공 true，실패 false
	 * @warning 실패 retval은 0，성공이라면 1
	 */
	bool hset( const std::string& key, const std::string& filed ,const std::string& value, uint32_t& retval )
	{
		if (!_connected || !_redCtx)
		{
			_errStr = _errDes[ERR_NO_CONNECT];
			return false;
		}

		bool ret = false;
		redisReply* reply = redisCmd("HSET %s %s %s", key.c_str(), filed.c_str(), value.c_str());

		if (_getError(reply))
		{
			ret = false;
		}
		else
		{
			retval = (uint32_t)reply->integer;
			ret = true;
		}

		if (NULL != reply)
		{
			freeReplyObject(reply);
		}

		return ret;
	}

	/**
	 * @brief 해시 테이블의 키에 해당하는 필드를 삭제한다.
	 * @param [in] key 테이블 이름과 같은 키 이름
	 * @param [in] filed 필드 이름
	 * @param [out] retval：삭제된 필드 수
	 * @return 성공 true，실패 false
	 *@warning 실패는 retval 0，성공이라면 1
	 */
	bool hdel(const std::string& key, const std::string& filed, uint32_t& retval)
	{
		{
			if (!_connected || !_redCtx)
			{
				_errStr = _errDes[ERR_NO_CONNECT];
				return false;
			}

			bool ret = false;
			redisReply* reply = redisCmd("HDEL %s %s", key.c_str(), filed.c_str(), retval);

			if (_getError(reply))
			{
				ret = false;
			}
			else
			{
				if (REDIS_REPLY_INTEGER == reply->type && 0 == reply->integer)
				{
					_errStr = std::string(_errDes[ERR_NO_KEY]) + " or " + _errDes[ERR_NO_FIELD];
				}
				//std::cout<<"type = "<<reply->type<<"   integer = "<< reply->integer<<std::endl;
				//std::cout<<"str = " << reply->str<<std::endl;
				else
				{
					retval = (uint32_t)reply->integer;
					ret = true;
				}
			}

			if (NULL != reply)
			{
				freeReplyObject(reply);
			}

			return ret;
		}
	}

	/**
	 * @brief 해시 테이블의 키에 해당하는 모든 내용을 가져온다
	 * @param [in] key 테이블 이름과 같은 키 이름이다.
	 * @param [out] valueMap 값（map<string,string> 유형）
	 * @return 성공 true，실패 false
	 * @warning 실패이면 valueMap은 비어 있다
	 */
	bool hgetall(const std::string& key, ValueMap& valueMap)
	{
		if (!_connected || !_redCtx)
		{
			_errStr = _errDes[ERR_NO_CONNECT];
			return false;
		}

		bool ret = false;
		redisReply* reply = redisCmd("HGETALL %s", key.c_str());

		if (_getError(reply))
		{
			ret = false;
		}
		else
		{

			if (REDIS_REPLY_ARRAY == reply->type && 0 == reply->elements)
			{
				_errStr = _errDes[ERR_NO_KEY];

			}
			else
			{
				_getArryToMap(reply, valueMap);
				ret = true;
			}
		}

		if (NULL != reply)
		{
			freeReplyObject(reply);
		}

		return ret;
	}

protected:
	/**
	 *@brief REDIS_REPLY_ARRY 유형의 reply->type 요소에서 데이터를 가져 와서 valueList 목록에 채운다.
	 *
	 *@param [in] reply  . the data return from redis-server.
	 *@param [out] valueList. reply에서 추출된 여러 데이터
	 *@return 성공 true, 실패　false.
	 */
	bool _getArryToList( redisReply* reply , ValueList& valueList )
	{
		if (NULL == reply)
		{
			return false;
		}

		std::size_t num = reply->elements;

		for (std::size_t i = 0; i < num; i++)
		{
			valueList.push_back(reply->element[i]->str);
		}

		//	ValueList::iterator it = valueList.begin();
		//
		//	for( ; it != valueList.end(); it++ )
		//	{
		//		std::cout << "valueList: "<< *it << std::endl;
		//	}
		return true;
	}

	/**
	 *@brief REDIS_REPLY_ARRY 유형의 reply->type 요소에서 데이터를 가져 와서 valueMap 목록을 채운다.
	 *
	 *hgetall은 홀수를 필드 이름으로, 짝수를 필드 데이터로 반환한다. 필드 이름 = 필드 데이터로 valueMap에 저장된다.
	 *
	 *@param [in] reply  . the data return from redis-server.
	 *@param [out] valueMap. reply에서 추출된 많은 페어 데이터는 valueMap에 저장된다.
	 *@return 성공 true, 실패　false.
	 */
	bool _getArryToMap( redisReply* reply , ValueMap& valueMap )
	{
		if (NULL == reply)
		{
			return false;
		}

		std::size_t num = reply->elements;

		for (std::size_t i = 0; i < num; i += 2)
		{
			valueMap.insert(
				std::pair<std::string, std::string>(reply->element[i]->str,
					reply->element[i + 1]->str));
		}

		return true;
	}

	bool _getError( const redisContext* redCtx )
	{
		_errStr = _errDes[ERR_NO_ERROR];
		if (redCtx == NULL)
		{
			_errStr = _errDes[ERR_NULL];
			return true;
		}
		if (redCtx->err != 0)
		{
			_errStr = redCtx->errstr;
			return true;
		}
		else
		{
			return false;
		}
	}

	bool _getError( const redisReply* reply )
	{
		_errStr = _errDes[ERR_NO_ERROR];
		if (reply == NULL)
		{
			_errStr = _errDes[ERR_NULL];
			return false;
		}

		// have error
		if (reply->type == REDIS_REPLY_ERROR)
		{
			_errStr = reply->str;
			return true;
		}
		else
		{
			return false;
		}
	}

	inline redisContext* _getCtx( ) const
	{
		return _redCtx;
	}

private:
	DISALLOW_COPY_AND_ASSIGN( CRedisConn );

	redisContext *_redCtx;		///< redis connector context

	std::string _host;         		///< redis host
	uint16_t _port;         		///< redis sever port
	std::string _password;         	///< redis server password
	uint32_t _timeout;      		///< connect timeout second
	bool _connected;			///< if connected

	///< error number
	enum E_ERROR_NO
	{
		ERR_NO_ERROR = 0,
		ERR_NULL,
		ERR_NO_CONNECT,
		ERR_NO_PIVOT,
		ERR_NO_KEY,
		ERR_NO_FIELD,
		ERR_INDEX,
		ERR_BOTTOM
	};
	std::string _errStr;		///< Describe the reason for error..

	static const char* _errDes[ERR_BOTTOM];	///< describe error
};

const char* CRedisConn::_errDes[ERR_BOTTOM] =
{
		"No error.",
		"NULL pointer ",
		"No connection to the redis server.",
				"Inser Error,pivot not found.",
				"key not found",
		"hash field not found",
		"error index"
};

} /* namespace RedisCpp */


