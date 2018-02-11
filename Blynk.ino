BLYNK_READ(V0)
{
  Blynk.virtualWrite(0, pan.read());
}

BLYNK_READ(V1)
{
  Blynk.virtualWrite(1, tilt.read());
}

BLYNK_WRITE(V0)
{
  //pan.attach(PAN);
  pan.write(param.asInt());
  //pan.detach();
  BLYNK_LOG("Moved Pan to : %s", param.asStr());
}

BLYNK_WRITE(V1)
{
  //tilt.attach(TILT);
  tilt.write(param.asInt());
  //tilt.detach();
  BLYNK_LOG("Moved Tilt to : %s", param.asStr());
}
