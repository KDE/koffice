
install( FILES  BMI.kst MenuPlan.kst VacationChecklist.kst CreditCardTracker.kst  DESTINATION ${DATA_INSTALL_DIR}/kcells/templates/HomeFamily/.source)
install( FILES  .directory BMI.desktop MenuPlan.desktop VacationChecklist.desktop CreditCardTracker.desktop  DESTINATION ${DATA_INSTALL_DIR}/kcells/templates/HomeFamily)

kde4_install_icons( ${DATA_INSTALL_DIR}/kcells/icons  )

