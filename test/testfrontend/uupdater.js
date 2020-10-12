const Eos = require('eosjs');
const dotenv = require('dotenv');
const axios = require('axios');

const url = "https://min-api.cryptocompare.com/data/price?fsym=EOS&tsyms=USD";

dotenv.load();

const interval = process.env.FREQ;
const owner = process.env.ORACLE;
const oracleContract = process.env.CONTRACT;

const oraclize = "oraclebosbos";
const consumer = "consumer1234";
const oracle = "oracleoracle";

const eos = Eos({ 
  httpEndpoint: process.env.EOS_PROTOCOL + "://" +  process.env.EOS_HOST + ":" + process.env.EOS_PORT,
  keyProvider: [process.env.EOS_KEY,'5JhNVeWb8DnMwczC54PSeGBYeQgjvW4SJhVWXMXW7o4f3xh7sYk','5JBqSZmzhvf3wopwyAwXH5g2DuNw9xdwgnTtuWLpkWP7YLtDdhp','5JCtWxuqPzcPUfFukj58q8TqyRJ7asGnhSYvvxi16yq3c5p6JRG','5K79wAY8rgPwWQSRmyQa2BR8vPicieJdLCXL3cM5Db77QnsJess'],
  chainId: process.env.EOS_CHAIN,
  verbose:false,
  logger: {
    log: null,
    error: null
  }
});

// async function getNewPermissions(accountName) {
// 	const account = await eos.getAccount(accountName)
// 	const perms = JSON.parse(JSON.stringify(account.permissions))
// 	return perms
//   }
  
  
//   const perms = await getNewPermissions(accountName)
//   console.log('New permissions =>', JSON.stringify(perms))
//   const updateAuthResult = await eos.transaction(tr => {
  
// 		for(const perm of perms) {
  
// 		   tr.updateauth({
// 			   account: accountName,
// 			   permission: perm.perm_name,
// 			   parent: perm.parent,
// 			   auth: perm.required_auth
// 		   }, {authorization: `${accountName}@owner`})
  
// 	   }
//   })
  
  
//   console.log('Success =>', JSON.stringify(updateAuthResult));


const require_permissions = ({ account, key, actor, parent }) => {
	return {
	  account: `${account}`,
	  permission: "active",
	  parent: `${parent}`,
	  auth: {
		threshold: 1,
		keys: [
		  {
			key: `${key}`,
			weight: 1
		  }
		],
		accounts: [
		  {
			permission: {
			  actor: `${actor}`,
			  permission: "eosio.code"
			},
			weight: 1
		  }
		],
		waits: []
	  }
	};
  };
  
  const allowContract = (auth, key, contract, parent) => {
	let [account, permission] = auth.split("@");
	permission = permission || "active";
	parent = parent || "owner";
  
	const tx_data = {
	  actions: [
		{
		  account: "eosio",
		  name: "updateauth",
		  authorization: [
			{
			  actor: account,
			  permission: permission
			}
		  ],
		  data: require_permissions({
			account: account,
			key: key,
			actor: contract,
			parent: parent
		  })
		}
	  ]
	};
  
	return tx_data;
  };

  const pub = "EOS89PeKPVQG3f48KCX2NEg6HDW7YcoSracQMRpy46da74yi3fTLP";
  eos.transaction(allowContract(consumer, pub, consumer));
//   await oraclizeContract.setup(oraclizeAccount, oracle, masterAccount, {
// 	authorization: [oraclizeAccount]
//   });

function  setup(){
	
	eos.contract(consumer)
					.then((contract) => {
						// const price = {
						// 	value: 200000,
						// 	decimals: 4
						// };
						// const priceBinary = contract.fc.toBuffer("price", price);
						contract.setup(oraclize, {
							 	authorization: [consumer]
							   }).then(results=>{
							console.log("results:", results);
							
						})
						.catch(error=>{
							console.log("error:", error);
						
						});
						
					})
					.catch(error=>{
						console.log("error:", error);
						setTimeout(write, interval);
					});

}
setup();

function addoracle(){
	
	eos.contract(oraclize)
					.then((contract) => {
						// const price = {
						// 	value: 200000,
						// 	decimals: 4
						// };
						// const priceBinary = contract.fc.toBuffer("price", price);
						contract.addoracle(owner, {
							authorization: [`${oraclize}@${process.env.ORACLE_PERMISSION || 'active'}`] 
						}).then(results=>{
							console.log("results:", results);
							
						})
						.catch(error=>{
							console.log("error:", error);
						
						});
						
					})
					.catch(error=>{
						console.log("error:", error);
						setTimeout(write, interval);
					});

}
addoracle();


  
function push(){
	
	eos.contract(oraclize)
					.then((contract) => {
						const price = {
							value: 300000,
							decimals: 4
						};
						const priceBinary = contract.fc.toBuffer("price", price);
						// contract.addoracle(owner, {
						// 	authorization: [`${oraclize}@${process.env.ORACLE_PERMISSION || 'active'}`] 
						// });
						contract.push(owner, consumer, "c0fe86756e446503eed0d3c6a9be9e6276018fead3cd038932cf9cc2b661d9de", "", priceBinary, {
							authorization: [`${owner}@${process.env.ORACLE_PERMISSION || 'active'}`] 
						}).then(results=>{
							console.log("results:", results);
							
						})
						.catch(error=>{
							console.log("error:", error);
						
						});
						
					})
					.catch(error=>{
						console.log("error:", error);
						setTimeout(write, interval);
					});

}
push();

function write(){

	axios.get(`${url}`)
		.then(results=>{

			if (results.data && results.data.USD){

				console.log(" results.data.USD",  results.data.USD);

				eos.contract(oracleContract)
					.then((contract) => {
						contract.write({
								owner: owner,
								value: parseInt(Math.round(results.data.USD * 10000))
							},
							{
								scope: oracleContract,
								authorization: [`${owner}@${process.env.ORACLE_PERMISSION || 'active'}`] 
							})
							.then(results=>{
								console.log("results:", results);
								setTimeout(write, interval);
							})
							.catch(error=>{
								console.log("error:", error);
								setTimeout(write, interval);
							});

					})
					.catch(error=>{
						console.log("error:", error);
						setTimeout(write, interval);
					});

			}
			else setTimeout(write, interval);

		})
		.catch(error=>{
			console.log("error:", error);
			setTimeout(write, interval);
		});

}


// write();

//setInterval(write, 60000);
