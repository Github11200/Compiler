define func with x of type integer gives back void as
  say x stop
end

define doSomething with x of type integer y of type integer gives back integer as
  call func with x stop
  call func with y stop

  for i less than 10 repeat
    say i stop
  end

  if y greater than x then
    give y back
  otherwise
    give x back
  end
end

say call doSomething with 5 comma 6 stop
