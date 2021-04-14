import {useState, useReducer} from 'react';
import './App.css';
import {CSSTransition, TransitionGroup} from 'react-transition-group'
import produce from 'immer';

const MAX_CONCURRENT_RUNNING = 1;

function ProcessOps(props: { dispatch: any; }) {
    const [pid, setPid] = useState(0);
    const {dispatch} = props;

    function add() {
        const timesliceRemaining = Math.ceil(Math.random()*10);
        dispatch({type: "ADD_READY_PROC", nextProc:{pid,timesliceRemaining}});
        setPid(e => e + 1);
    }

    return (
        <div id="processOps">
            <button type="button" onClick={() => dispatch({type: "NEXT_QUANTUM"})}>
                Next timeslice
            </button>
            <button type="button" onClick={add}>
                Add
            </button>
        </div>
    );
}

interface IProcess {
    pid:number;
    timesliceRemaining:number;
};

interface IBlockingProcess extends IProcess{
    blockingCounter:number;
};

function BlockingProcess(props:any) {
    const {p: {pid, timesliceRemaining, blockingCounter}, children, ...rest} = props;
    return (
        <div {...rest} style={{"--counter": blockingCounter}}>
            {pid}
            <div className="blockingCounter" title="阻塞还需几个时间片结束">
                {blockingCounter}
            </div>
            <div className="timesliceRemaining" title="还需运行几个时间片">
                {timesliceRemaining}
            </div>
        </div>
    )
}

function ProcessesView(props:any) {
    const {processes, title, children, ...rest} = props;
    return (
        <div className={`Processes`} {...rest}>
            <div className="processHeader">{children}</div>
            <TransitionGroup component={null} exit={false}>
                {
                    title === "blocking" ?
                        processes.map((p:IBlockingProcess) => (
                            <CSSTransition key={p.pid} timeout={500} classNames="process">
                                <BlockingProcess className="process blocking" p={p}/>
                            </CSSTransition>
                        ))
                        :
                        processes.map(({pid,timesliceRemaining}:IProcess) => (
                            <CSSTransition key={pid} timeout={500} classNames="process">
                                <div className={`process ${title}`}>
                                    {pid}
                                    <div className="timesliceRemaining" title="还需运行几个时间片">
                                        {timesliceRemaining}
                                    </div>
                                </div>
                            </CSSTransition>
                        ))
                }
            </TransitionGroup>
        </div>
    );
}

interface OSState {
    runningProcesses: IProcess[];
    readyProcesses: IProcess[];
    blockingProcesses: IBlockingProcess[];
}

/*
                              ┌-------------------┐
                              │                   ↓
三状态   start--->ready<-->running--->blocking   end
                    ↑                  │
                    └---------------- -┘

        blocking:blockingCounter-1
        running:go blocking or ready
        readying: some go running.
*/
const OSInitialState: OSState = {
    runningProcesses: [],
    readyProcesses: [],
    blockingProcesses: []
};

type NormalAction = {
    type: "NEXT_QUANTUM"
}
type AddAction = NormalAction | {
    type: "ADD_READY_PROC"
    nextProc: IProcess
}

const OSScheduler = produce((draft:OSState, action:NormalAction|AddAction) => {
    const {
        runningProcesses: runningProcs,
        readyProcesses: readyProcs
    } = draft;
    switch (action.type) {
        case "NEXT_QUANTUM": {
            const blockingProcs = draft.blockingProcesses.filter(p => {
                --p.blockingCounter;
                const ok = p.blockingCounter > 0;
                if (!ok) {
                    const {blockingCounter, ...okp} = p;
                    readyProcs.push(okp);
                }
                return ok;
            });

            runningProcs.forEach(p => {
                --p.timesliceRemaining;
                if(p.timesliceRemaining>0){
                    const goBlocked = Math.random() > 0.5;
                    if (goBlocked) {
                        const bp = (p as IBlockingProcess);
                        bp.blockingCounter = Math.ceil(Math.random() * 10);
                        blockingProcs.push(bp);
                    } else {
                        readyProcs.push(p);
                    }
                }
            });

            draft.blockingProcesses = blockingProcs;
            draft.runningProcesses = readyProcs.splice(0, MAX_CONCURRENT_RUNNING);
            draft.blockingProcesses = blockingProcs.sort((p1, p2) => p1.blockingCounter - p2.blockingCounter);
            return;
        }
        case "ADD_READY_PROC": {
            readyProcs.push(action.nextProc);
            return;
        }
        default:
            throw new Error();
    }
});


function App() {
    const [processes, dispatch] = useReducer(OSScheduler, OSInitialState);

    return (
        <div className="App">
            <ProcessesView processes={processes.runningProcesses} title="running">
                Running
            </ProcessesView>
            <ProcessesView processes={processes.readyProcesses} title="ready">
                Ready
            </ProcessesView>
            <ProcessesView processes={processes.blockingProcesses} title="blocking">
                Blocking
            </ProcessesView>
            <ProcessOps {...{dispatch}}/>
        </div>
    );
}

export default App;
