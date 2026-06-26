DynamicProtocol::ParsedFrame frame =
        DynamicProtocol::parseFrame(canId, data, m_seed);

if (frame.valid && (frame.seq == 45 || frame.seq == 49)) {
    qDebug().noquote()
            << QString("[SEQ45_49_RAW] product=%1 channel=%2 canId=%3 raw=[%4] "
                       "seq=%5 seed=%6 value=%7 groupCnt=%8 checkValue=%9 "
                       "valid=%10 seedOk=%11 checkOk=%12 activeGroup=%13 detectProgress=%14")
               .arg(m_productCode)
               .arg(m_channelConfig.name)
               .arg(DynamicProtocol::canIdToString(canId))
               .arg(DynamicProtocol::frameDataToString(data))
               .arg(frame.seq)
               .arg(frame.seed)
               .arg(frame.value)
               .arg(frame.groupCnt)
               .arg(frame.checkValue)
               .arg(frame.valid ? "true" : "false")
               .arg(frame.seedOk ? "true" : "false")
               .arg(frame.checkOk ? "true" : "false")
               .arg(m_activeGroupCnt)
               .arg(m_detectProgress);
}
