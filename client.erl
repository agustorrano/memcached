-module(client).
-export([start/1, put/3, get/2, del/2, stats/1]).

-define(PUT, 11).
-define(DEL, 12).
-define(GET, 13).
-define(STATS, 21).
-define(OK, 101).
-define(EINVAL, 111).
-define(ENOTFOUND, 112).
-define(EBINARY, 113).
-define(EBIG, 114).
-define(EUNK, 115).
-define(EOOM, 116).

% Crea un proceso que va a representar a un cliente el cual le
% enviará mensajes al servidor y esperará respuestas.
start(HostName) ->
  case gen_tcp:connect(HostName, 8889, [binary, {active, false}]) of
    {ok, Socket} -> spawn(fun() -> client(Socket) end);
    {error, Reason} -> Reason
  end.


client(Socket) ->
  receive
    {put, K, V, Pid} -> Res = send_put(Socket, K, V),
                        Pid ! Res;
    {get, K, Pid} -> Res = send_get(Socket, K),
                     Pid ! Res;
    {del, K, Pid} -> Res = send_del(Socket, K),
                     Pid ! Res;
    {stats, Pid} -> Res = send_stats(Socket),
                    Pid ! Res
  end,
  client(Socket).


encode(Input) ->
  BInput = term_to_binary(Input),
  Size = byte_size(BInput),
  BSize = <<Size:32>>,
  <<BSize/binary, BInput/binary>>.


decode(Cmd) ->
  case Cmd of
    <<?OK>> -> ok;
    <<?EINVAL>> -> einval;
    <<?ENOTFOUND>> -> enotfound;
    <<?EBINARY>> -> ebinary;
    <<?EBIG>> -> ebig;
    <<?EUNK>> -> eunk;
    <<?EOOM>> -> eoom;
    _ -> error
  end.


answer(Socket) ->
  case gen_tcp:recv(Socket, 1) of
    {ok, Cmd} -> {ok, decode(Cmd)};
    Error -> Error
  end.


answer_data(Socket) ->
  case gen_tcp:recv(Socket, 4) of
    {ok, BSize} -> gen_tcp:recv(Socket, binary:decode_unsigned(BSize));
    Error -> Error
  end.


send_put(Socket, K, V) ->
  BKey = encode(K),
  BValue = encode(V),
  Req = <<?PUT, BKey/binary, BValue/binary>>,
  gen_tcp:send(Socket, Req),
  answer(Socket).


send_get(Socket, K) ->
  BKey = encode(K),
  Req = <<?GET, BKey/binary>>,
  gen_tcp:send(Socket, Req),
  case answer(Socket) of
    {ok, ok} -> 
      case answer_data(Socket) of
        {ok, BValue} -> {ok, ok, binary_to_term(BValue)};
        Error -> Error
      end;
    {ok, Cmd} -> {ok, Cmd};
    Error -> Error
  end.


send_del(Socket, K) ->
  BKey = encode(K),
  Req = <<?DEL, BKey/binary>>,
  gen_tcp:send(Socket, Req),
  answer(Socket).


send_stats(Socket) ->
  Req = <<?STATS>>,
  gen_tcp:send(Socket, Req),
  case answer(Socket) of
    {ok, ok} -> 
      case answer_data(Socket) of
        {ok, Bin} -> {ok, ok, binary_to_list(Bin)};
        Error -> Error
      end;
    {ok, Cmd} -> {ok, Cmd};
    Error -> Error
  end.


put(Id, K, V) ->
  Id ! {put, K, V, self()}.

get(Id, K) ->
  Id ! {get, K, self()}.

del(Id, K) ->
  Id ! {del, K, self()}.

stats(Id) ->
  Id ! {stats, self()}.
