-module(client).
-export([start/1, put/3, get/2, del/2, stats/1, close/1]).

% MACROS
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
  case gen_tcp:connect(HostName, 8889, [binary, {active, false}, {packet, raw}]) of
    {ok, Socket} -> spawn(fun() -> client(Socket) end);
    {error, Error} -> {error, Error}
  end.


client(Socket) ->
  receive
    {put, K, V, Pid} -> Pid ! send_put(Socket, K, V);
    {get, K, Pid} -> Pid ! send_get(Socket, K);
    {del, K, Pid} -> Pid ! send_del(Socket, K);
    {stats, Pid} -> Pid ! send_stats(Socket);
    close -> gen_tcp:close(Socket)
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
    _ -> unknown
  end.


answer(Socket) ->
  case gen_tcp:recv(Socket, 1) of
    {ok, Cmd} -> decode(Cmd);
    {error, Error} -> {error, Error}
  end.


answer_data(Socket) ->
  case gen_tcp:recv(Socket, 4) of
    {ok, BSize} -> gen_tcp:recv(Socket, binary:decode_unsigned(BSize));
    {error, Error} -> {error, Error}
  end.


send_put(Socket, K, V) ->
  BKey = encode(K),
  BValue = encode(V),
  Req = <<?PUT, BKey/binary, BValue/binary>>,
  case gen_tcp:send(Socket, Req) of
    ok -> answer(Socket);
    {error, Error} -> {error, Error}
  end.


send_get(Socket, K) ->
  BKey = encode(K),
  Req = <<?GET, BKey/binary>>,
  case gen_tcp:send(Socket, Req) of
    ok -> 
      case answer(Socket) of
        ok -> 
          case answer_data(Socket) of
            {ok, BValue} -> {ok, binary_to_term(BValue)};
            {error, Error} -> {error, Error}
          end;
        Cmd -> Cmd
      end;
    {error, Error} -> {error, Error}
  end.


send_del(Socket, K) ->
  BKey = encode(K),
  Req = <<?DEL, BKey/binary>>,
  case gen_tcp:send(Socket, Req) of
    ok -> answer(Socket);
    {error, Error} -> {error, Error}
  end.


send_stats(Socket) ->
  Req = <<?STATS>>,
  case gen_tcp:send(Socket, Req) of
    ok ->
      case answer(Socket) of
        ok -> 
          case answer_data(Socket) of
            {ok, Bin} -> {ok, binary_to_list(Bin)};
            {error, Error} -> {error, Error}
          end;
        Cmd -> Cmd
      end;
    {error, Error} -> {error, Error}
  end.


put(Id, K, V) ->
  Id ! {put, K, V, self()},
  receive
    Ans -> Ans
  end.

get(Id, K) ->
  Id ! {get, K, self()},
  receive
    Ans -> Ans
  end.

del(Id, K) ->
  Id ! {del, K, self()},
  receive
    Ans -> Ans
  end.

stats(Id) ->
  Id ! {stats, self()},
  receive
    Ans -> Ans
  end.

close(Id) -> Id ! close.
