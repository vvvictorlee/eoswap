import React, { useEffect, useState } from 'react'
import { Grid, Label } from 'semantic-ui-react'
import { useSubstrateState } from './substrate-lib'

export default function Main(props) {
  const { contract, currentAccount } = useSubstrateState()
  const [metrics, setMetrics] = useState({
    totalSupply: '0',
    totalBalance: '0',
  })
  const { totalSupply, totalBalance } = metrics
  useEffect(() => {
    let unsub = null
    const asyncFetch = async () => {
      if (currentAccount == null || contract == null) {
        return
      }

      let metricsMap = {}
      let { output } = await contract['mgmt'].query['metrics'](
        currentAccount.address,
        {
          value: 0,
          gasLimit: -1,
        },
        currentAccount.address
      )
      metricsMap = {
        totalSupply: output[0].toString(),
        totalBalance: output[1].toString(),
      }
      setMetrics(metricsMap)
    }

    asyncFetch()

    return () => {
      unsub && unsub()
    }
  }, [contract, currentAccount, setMetrics])

  return (
    <Grid.Column>
      <h1>Metrics</h1>
      <Label basic color="teal">
        {totalSupply} followers
      </Label>
      <Label basic color="teal" style={{ marginLeft: 0, marginTop: '.5em' }}>
        {totalBalance} followings
      </Label>
    </Grid.Column>
  )
}
