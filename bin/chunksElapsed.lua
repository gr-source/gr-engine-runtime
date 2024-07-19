

function Main()
    local ChunksElapsed = {}

    function ChunksElapsed:Init()
        print("Initialize Script");
    end

    function ChunksElapsed:Start()
        print("Started GameObject");
    end

    function ChunksElapsed:Update()
        print("Update GameObject");
    end

    function ChunksElapsed:Destroy()
        print("GameObject Destroyed");
    end

    return ChunksElapsed
end